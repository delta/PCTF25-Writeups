<?php
class Auth {
    public function register($username, $password) {
        if (!isset($_SESSION['users'])) {
            $_SESSION['users'] = [];
        }

        if (isset($_SESSION['users'][$username])) {
            return ['error' => 'User already exists'];
        }

        // Hash and store the password in the session
        $_SESSION['users'][$username] = password_hash($password, PASSWORD_DEFAULT);
        return ['message' => 'Registration successful'];
    }

    public function login($username, $password) {
        if (!isset($_SESSION['users'][$username])) {
            return ['error' => 'User not found'];
        }

        if (!password_verify($password, $_SESSION['users'][$username])) {
            return ['error' => 'Invalid password'];
        }

        // Create a session variable for the logged-in user
        $role = ($username === 'admin') ? 'admin' : 'user'; // Assign role based on username
        $_SESSION['auth'] = [
            'username' => $username,
            'role' => $role,
            'exp' => time() + 3600 // Expire session after 1 hour
        ];

        // If the user is "admin", also issue a JWT for CTF purposes
        if ($role === 'admin') {
            $payload = [
                'username' => $username,
                'role' => 'user',
                'exp' => time() + 3600
            ];

            $token = jwt_encode($payload, JWT_SECRET, 'HS256');
            setcookie('auth_token', $token, [
                'expires' => time() + 3600,
                'path' => '/',
                'httponly' => true,
                'secure' => false,
                'samesite' => 'Strict'
            ]);
        }

        return ['message' => 'Login successful'];
    }


    public function logout() {
        // Clear session data
        unset($_SESSION['auth']);


        setcookie('auth_token', '', [
            'expires' => time() - 3600,
            'path' => '/',
            'httponly' => true,
            'secure' => false,
            'samesite' => 'Strict'
        ]);

        return ['message' => 'Logged out successfully'];
    }
}
?>
