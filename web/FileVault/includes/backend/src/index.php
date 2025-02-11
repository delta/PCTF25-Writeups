<?php
require_once 'config.php';
require_once 'auth.php';
require_once 'upload.php';
require_once 'Verifier.php';
require_once 'internal_verify.php';
require_once 'download_file.php';
require 'vendor/autoload.php';
use Firebase\JWT\JWT;
use Firebase\JWT\Key;


$route = rtrim(str_replace('/index.php', '', parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH)), '/');
$method = $_SERVER['REQUEST_METHOD'];

switch ($route) {

    case '':
        if ($method !== 'GET') {
            http_response_code(405);
            echo json_encode(['error' => 'Method not allowed']);
            break;
        }
        
        $get = $_GET['get'] ?? null;
    
        if (!$get) {
            http_response_code(400);
            echo json_encode(['error' => 'path is incomplete']);
            break;
        }
        if ($get == 'help') {
            echo "soon!!";
            break;
        }
        else if ($get == 'license') {
            echo "no license yet";
            break;
        }
        else {
            echo "huh?";
            break;
        }
        break;


    case '/v1':
        if (!isset($_SESSION['auth'])) {
            header("Location: /login");
            exit;
        }
        include 'dashboard.html';
        break;

    case '/v1/login':
        if (isset($_SESSION['auth'])) {
            header("Location: /");
            exit;
        }
        include 'login.html';
        break;

    case '/v1/register':
        if (isset($_SESSION['auth'])) {
            header("Location: /");
            exit;
        }
        include 'register.html';
        break;

    case '/v1/api/check-auth':
        if (!isset($_SESSION['auth'])) {
            http_response_code(401);
            echo json_encode(['error' => 'Not authenticated']);
        } else {
            echo json_encode(['success' => true]);
        }
        break;

    case '/v1/api/register':
        if ($method !== 'POST') {
            http_response_code(405);
            echo json_encode(['error' => 'Method not allowed']);
            break;
        }
        $data = json_decode(file_get_contents('php://input'), true);
        $auth = new Auth();
        $result = $auth->register($data['username'], $data['password']);
        
        if (isset($result['error'])) {
            http_response_code(400);
            echo json_encode($result);
        } else {
            $loginResult = $auth->login($data['username'], $data['password']);
            if (isset($loginResult['error'])) {
                http_response_code(401);
                echo json_encode($loginResult);
            } else {
                http_response_code(200);
                echo json_encode(['success' => true, 'message' => 'Registration and login successful']);
            }
        }
        break;

    case '/v1/api/logout':
        $auth = new Auth();
        echo json_encode($auth->logout());
        header("Location: /login");
        break;

    case '/v1/api/login':
        if ($method === 'POST') {
            $data = json_decode(file_get_contents('php://input'), true);
            $auth = new Auth();
            $result = $auth->login($data['username'], $data['password']);
            if (isset($result['error'])) {
                http_response_code(401);
                echo json_encode($result);
            } else {
                http_response_code(200);
                echo json_encode(['success' => true]);
            }
        }
        break;

    case '/v1/api/upload':
        if ($method === 'POST') {
            $uploader = new FileUploader();
            echo json_encode($uploader->upload($_FILES['file']));
        }
        break;

    case '/internal/verify':
        if ($method !== 'POST') {
            http_response_code(405);
            echo json_encode(['error' => 'Method not allowed']);
            break;
        }
    
        // Read JSON from the request body
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

    case '/v1/api/files':
        if ($method !== 'GET') {
            http_response_code(405);
            echo json_encode(['error' => 'Method not allowed']);
            break;
        }
    
        session_start();
    
        // Check if the user is authenticated
        if (!isset($_SESSION['auth'])) {
            http_response_code(401);
            echo json_encode(['error' => 'Unauthorized']);
            break;
        }
    
        // Get the username of the authenticated user
        $username = $_SESSION['auth']['username'];
    
        // Check if there are any files stored in the session
        if (!isset($_SESSION['files']) || empty($_SESSION['files'])) {
            echo json_encode([]); // Return an empty array if no files exist
            break;
        }
    
        // Filter files belonging to the authenticated user
        $userFiles = [];
        foreach ($_SESSION['files'] as $fileId => $fileInfo) {
            if ($fileInfo['user'] === $username) {
                $userFiles[] = [
                    'id' => $fileId,
                    'name' => $fileInfo['name'],
                    'uploadedAt' => $fileInfo['uploadedAt'] ?? null
                ];
            }
        }
    
        // Return the filtered files as JSON
        echo json_encode($userFiles);
        break;

    case (preg_match('/^\/v1\/api\/files\/([^\/]+)\/download$/', $route, $matches) ? true : false):
        if ($method !== 'GET') {
            http_response_code(405);
            echo json_encode(['error' => 'Method not allowed']);
            break;
        }
    
        $fileId = $matches[1]; // Extract the file ID from the URL
        handleFileDownload($fileId);
        break;


    case '/v1/api/flag':
        if ($method !== 'GET') {
            http_response_code(405);
            echo json_encode(['error' => 'Method not allowed']);
            break;
        }
        $headers = getallheaders();
        $auth_header = $headers['Authorization'] ?? '';
        if (!preg_match('/Bearer\s+(.*)$/i', $auth_header, $matches)) {
            http_response_code(401);
            echo json_encode(['error' => 'No token provided']);
            break;
        }
        $token = $matches[1];
        try {
            $payload = JWT::decode($token, new Key(JWT_SECRET, 'HS256'));
            if (isset($payload->role) && $payload->role === 'admin') {
                echo "YOU GOT IT!!! p_ctf{THe_mASt9r_0f_HRS_Ju5t_KiDdIng}";
            } else {
                echo json_encode(['error' => 'Access denied. You are not an admin.']);
            }
        } catch (Exception $e) {
            http_response_code(401);
            echo json_encode(['error' => 'Invalid token', 'message' => $e->getMessage()]);
        }
        break;

    default:
        http_response_code(404);
        echo json_encode(['error' => 'Not found']);
        break;
}
?>
