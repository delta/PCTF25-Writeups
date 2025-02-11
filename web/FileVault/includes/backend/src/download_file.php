<?php

function handleFileDownload($fileId) {
    session_start();

    // Check if the user is authenticated
    if (!isset($_SESSION['auth'])) {
        http_response_code(401); // Unauthorized
        echo json_encode(['error' => 'Unauthorized']);
        return;
    }

    // Check if the file exists in the session
    if (!isset($_SESSION['files'][$fileId])) {
        http_response_code(404); // File not found
        echo json_encode(['error' => 'File not found']);
        return;
    }

    $file = $_SESSION['files'][$fileId]; // Fetch the file metadata

    // Decode the file content stored in base64
    $fileContent = base64_decode($file['content']);
    if ($fileContent === false) {
        http_response_code(500); // Internal Server Error
        echo json_encode(['error' => 'Failed to retrieve file content']);
        return;
    }

    // Set headers to force a file download
    header('Content-Type: application/octet-stream');
    header('Content-Disposition: attachment; filename="' . basename($file['name']) . '"');
    header('Content-Length: ' . strlen($fileContent));

    // Output the file content
    echo $fileContent;
}

?>
