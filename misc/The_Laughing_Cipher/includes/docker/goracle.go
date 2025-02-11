package main

import (
    "encoding/hex"
    "log"
    "math/big"
    "net/http"

    "github.com/gin-gonic/gin"
)

// Global constants and variables
const (
    ModulusSize = 1024
    K           = ModulusSize / 8
)

var (
    sk = struct {
        n *big.Int
        d *big.Int
    }{
        n: new(big.Int),
        d: new(big.Int),
    }
)

func init() {
    // Initialize the big integers from strings
    sk.n.SetString("90755263541684588974054530792483677455898840942739210225029812710552341527556120481051645722505814811089681839278418335307312601942090489438507544120211485428365857239966842965248967998078147498280578516594079083591608859724969009400595378977246756348937112658062561728758753393034769353279956927820641491047", 10)
    sk.d.SetString("60503509027789725982703020528322451637265893961826140150019875140368227685037413654034430481670543207393121226185612223538208401294726992959005009124673503113061689980062375066515865534128637250297282305449017081273082962170711937014984463790519374370508154889864773717882004205789901503277093131240490317307", 10)
}

// Request and response structures
type OracleRequest struct {
    Ciphertext string `json:"ciphertext" binding:"required"`
    N big.Int `json:"N" binding:"required"`
    e int `json:"e" binding:"required` // hex-encoded ciphertext
}

type OracleResponse struct {
    Valid   bool   `json:"valid"`
    Error   string `json:"error,omitempty"`
    Message string `json:"message,omitempty"`
}

// Utility functions
func integerToBytes(integer *big.Int) []byte {
    bytes := integer.Bytes()
    if len(bytes) < K {
        padded := make([]byte, K)
        copy(padded[K-len(bytes):], bytes)
        return padded
    }
    return bytes
}

func decryptInteger(secretKey struct{ n, d *big.Int }, c *big.Int) *big.Int {
    return new(big.Int).Exp(c, secretKey.d, secretKey.n)
}

func decryptString(secretKey struct{ n, d *big.Int }, ciphertext []byte) []byte {
    encInteger := new(big.Int).SetBytes(ciphertext)
    integer := decryptInteger(secretKey, encInteger)
    return integerToBytes(integer)
}

func oracle(ciphertext []byte) bool {
    encoded := decryptString(sk, ciphertext)

    if len(encoded) > K {
        return false
    }

    if len(encoded) < K {
        zeroPad := make([]byte, K-len(encoded))
        encoded = append(zeroPad, encoded...)
    }

    return len(encoded) >= 2 && encoded[0] == 0x00 && encoded[1] == 0x02
}

// Gin handler
func handleDecrypt(c *gin.Context) {
    var req OracleRequest
    if err := c.ShouldBindJSON(&req); err != nil {
        c.JSON(http.StatusBadRequest, OracleResponse{
            Valid: false,
            Error: "Invalid request format",
        })
        return
    }

    // Decode hex string
    ciphertext, err := hex.DecodeString(req.Ciphertext)
    if err != nil {
        c.JSON(http.StatusBadRequest, OracleResponse{
            Valid: false,
            Error: "Invalid hex encoding",
        })
        return
    }

    // Process the decryption
    result := oracle(ciphertext)

    // Send response
    response := OracleResponse{
        Valid: result,
    }
    if result {
        response.Message = "Valid PKCS1 padding"
    } else {
        response.Message = "Invalid prefix"
    }

    c.JSON(http.StatusOK, response)
}

func setupRouter() *gin.Engine {
    // Set Gin to release mode
    gin.SetMode(gin.ReleaseMode)

    r := gin.Default()

    // Add middleware for logging and recovery
    r.Use(gin.Logger())
    r.Use(gin.Recovery())

    // API routes
    api := r.Group("/api")
    {
        api.POST("/decrypt", handleDecrypt)
    }

    return r
}

func main() {
    router := setupRouter()
    
    log.Println("Server starting on :8080...")
    log.Fatal(router.Run(":8080"))
}