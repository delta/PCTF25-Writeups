<?php
class FileUploader {
    public function upload($file) {
        // Check if the user is authenticated
        if (!isset($_SESSION['auth'])) {
            return ['error' => 'Unauthorized'];
        }

        // Check if a file was uploaded
        if (!isset($file['tmp_name']) || $file['tmp_name'] === '') {
            return ['error' => 'No file uploaded'];
        }

        // Read the file contents into a variable
        $fileContents = file_get_contents($file['tmp_name']);
        if ($fileContents === false) {
            return ['error' => 'Failed to read uploaded file'];
        }

        // Generate a unique file ID and store it in the session
        $username = $_SESSION['auth']['username'];
        $fileId = uniqid();
        if (!isset($_SESSION['files'])) {
            $_SESSION['files'] = [];
        }

        // Store file metadata and content in the session
        $_SESSION['files'][$fileId] = [
            'name' => $file['name'],               // Original file name
            'hash' => hash('sha256', $fileContents), // Hash of the file contents
            'content' => base64_encode($fileContents), // Store the file content as base64
            'user' => $username,                   // Username of the uploader
            'uploadedAt' => date('Y-m-d H:i:s')    // Upload timestamp
        ];

        // Return success response with file details
        return [
            'success' => true,
            'file_id' => $fileId,
            'name' => $file['name']
        ];
    }
}
?>
