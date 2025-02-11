function toggleForms() {
    const loginForm = document.getElementById('login-form');
    const registerForm = document.getElementById('register-form');
    
    if (loginForm && registerForm) {
        loginForm.classList.toggle('hidden');
        registerForm.classList.toggle('hidden');
    }
}

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

async function handleLogout() {
    try {
        await fetch('/api/logout', { method: 'POST' });
        document.getElementById('auth-section').classList.remove('hidden');
        document.getElementById('dashboard').classList.add('hidden');
        document.getElementById('content-display').textContent = '';
        document.getElementById('error-display').textContent = '';
    } catch (error) {
        console.error('Logout failed:', error);
    }
}

async function requestFile(fileId) {
    try {

        const requestData = {
            file_id: fileId
        };

        const response = await fetch('/api/files', {
            method: 'POST',
            credentials: 'include',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(requestData)
        });

        const data = await response.json();
        if (response.ok) {
            document.getElementById('content-display').textContent = data.content;
            document.getElementById('error-display').textContent = '';
        } else {
            document.getElementById('error-display').textContent = data.error;
            document.getElementById('content-display').textContent = '';
        }
    } catch (error) {
        document.getElementById('error-display').textContent = 'An error occurred while fetching the file';
        document.getElementById('content-display').textContent = '';
    }
}