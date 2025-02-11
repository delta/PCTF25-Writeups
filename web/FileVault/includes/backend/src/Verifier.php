<?php
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
?>