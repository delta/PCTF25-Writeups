const express = require('express');
const session = require('express-session');
const bodyParser = require('body-parser');
const crypto = require('crypto');
const fs = require('fs');
const path = require('path');
const rateLimit = require('express-rate-limit');

const app = express();

const apiLimiter = rateLimit({
    windowMs: 15 * 60 * 1000,
    max: 100,
    message: { error: 'Too many attempts, please try again later' }
});

app.use('/api/', apiLimiter);

app.use((req, res, next) => {
    res.setHeader(
        'Content-Security-Policy',
        "default-src 'self'; script-src 'self' 'unsafe-eval' 'unsafe-inline'; style-src 'self' 'unsafe-inline';"
    );
    next();
});

app.use(session({
    secret: crypto.randomBytes(32).toString('hex'),
    resave: false,
    saveUninitialized: true,
    cookie: {
        httpOnly: true,
        secure: false,
        sameSite: 'lax'
    }
}));

app.use(bodyParser.json({
    limit: '10kb',
    verify: (req, res, buf, encoding) => {
        const str = buf.toString();
        if (str.includes('__proto__') || 
            str.includes('constructor') || 
            str.includes('prototype')) {
            throw new Error('Invalid payload');
        }
    }
}));

app.use(express.static('public'));

const users = new Map();
const USER_TTL = 15 * 60 * 1000;

function setUser(email, userData) {
    const user = { ...userData, createdAt: Date.now() };
    users.set(email, user);
}

  // Periodically clean up expired users
setInterval(() => {
const now = Date.now();
for (const [email, user] of users.entries()) {
    if (now - user.createdAt > USER_TTL) {
    users.delete(email);
    console.log(`Deleted expired user: ${email}`);
    }
}
}, 60 * 1000); // runs every minute

class SecurityVerifier {
    #ref;
    hiddenKey;
    verifyFn;

    constructor({ hiddenKey }) {
        this.#ref = Object.freeze({
            state: false
        });
        
        this.hiddenKey = hiddenKey || 'ref_' + Math.random().toString(36).substr(2, 5);

        this.verifyFn = (r) => {
            try {
                const p = r[this.hiddenKey];
                if (p && p.auth === true) {
                    return true;
                }
                return false;
            } catch {
                return false;
            }
        };
    }

    resolve(base, ext) {
        const target = Object.create(null);

        Object.keys(base).forEach((key) => {
            if (base[key] && typeof base[key] === 'object') {
                target[key] = this.resolve(base[key], {});
            } else {
                target[key] = base[key];
            }
        });

        if (ext && typeof ext === 'object') {
            Object.keys(ext).forEach((key) => {
                const value = ext[key];
                if (value && typeof value === 'object') {
                    if (!target[key]) target[key] = {};
                    target[key] = this.resolve(target[key], value);
                } else {
                    target[key] = value;
                }
            });
        }

        return target;
    }

    verify(input) {
        try {
            const merged = this.resolve({}, input);
            if (typeof this.verifyFn === 'function') {
                return this.verifyFn(merged);
            }
            return false;
        } catch (error) {
            console.error('Verify error:', error);
            return false;
        }
    }
}


app.post('/api/register', async (req, res) => {
    const { email, password, publicKey} = req.body;

    if (!email || !password || 
        typeof email !== 'string' || 
        typeof password !== 'string' ||
        typeof publicKey !== 'string') {
        return res.status(400).json({ error: 'Invalid input' });
    }

    if (users.has(email)) {
        return res.status(400).json({ error: 'User already exists' });
    }

    const securityKey = 'ref_' + Math.random().toString(36).substr(2, 5);

    const user = {
        email,
        password: await hashPassword(password),
        securityKey,
        publicKey
    };

    setUser(email, user);
    res.json({ message: 'Registration successful' });
});

app.post('/api/login', async (req, res) => {
    const { email, password } = req.body;

    if (
        !email || !password ||
        typeof email !== 'string' ||
        typeof password !== 'string'
    ) {
        return res.status(400).json({ error: 'Invalid input' });
    }

    const user = users.get(email);
    if (!user) {
        return res.status(401).json({ error: 'Invalid credentials' });
    }

    const hashedPassword = await hashPassword(password);
    if (user.password !== hashedPassword) {
        return res.status(401).json({ error: 'Invalid credentials' });
    }

    try {
        const encryptedBuffer = crypto.publicEncrypt(
            {
                key: user.publicKey,
                padding: crypto.constants.RSA_PKCS1_OAEP_PADDING,
                oaepHash: "sha256"
            },
            Buffer.from(user.securityKey, 'utf8')
        );
        const encryptedSecurityKey = encryptedBuffer.toString('base64');
        req.session.user = {
            email: user.email,
            securityKey: user.securityKey
        };

        res.json({
            message: 'Login successful',
            securityKey: encryptedSecurityKey
        });
    } catch (err) {
        console.error('Encryption error:', err);
        res.status(500).json({ error: 'Internal server error' });
    }
});

app.post('/api/files', (req, res) => {
    if (!req.session.user) {
        return res.status(401).json({ error: 'Not authenticated' });
    }
    const fileName = req.body.file_id;
    if (typeof fileName !== 'string' || 
        !fileName.endsWith('.txt') || 
        fileName.includes('/') || 
        fileName.includes('\\') || 
        fileName.includes('..')) {
        return res.status(400).json({ error: 'Invalid filename' });
    }
    try {
        const publicFiles = ['welcome.txt', 'about.txt'];
        if(fileName === 'flag.txt') {
            const userKey = req.session.user.securityKey;
            const verifier = new SecurityVerifier({ hiddenKey: userKey });
            if (!verifier.verify(req.body.data)) {
                return res.status(403).json({ 
                    error: `Access Denied`,
                });
            }
        } else if(!publicFiles.includes(fileName)) {
            return res.status(404).json({ error: 'File not found' });
        }

        const filePath = path.join(__dirname, 'files', fileName);
        if (!fs.existsSync(filePath)) {
            return res.status(404).json({ error: 'File not found' });
        }

        const content = fs.readFileSync(filePath, 'utf8');
        res.json({ content });

    } catch (error) {
        res.status(500).json({ error: "error" });
    }
});

app.post('/api/logout', (req, res) => {
    req.session.destroy();
    res.json({ message: 'Logged out successfully' });
});

async function hashPassword(password) {
    return crypto.createHash('sha256')
        .update(password)
        .digest('hex');
}

app.use((err, req, res, next) => {
    res.status(500).json({ error: "error"});
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});