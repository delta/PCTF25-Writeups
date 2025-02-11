import hmac
import hashlib

class HiddenModule:
    """
    A hidden module that signs tokens using multiple layers of hashing.
    """
    def __init__(self, key):
        self._key = key

    def _hash_layer1(self, token):
        """
        First layer: Compute an HMAC-SHA256 of the token.
        """
        return hmac.new(self._key.encode(), token.encode(), hashlib.sha256).hexdigest()

    def _hash_layer2(self, data):
        """
        Second layer: Reverse the previous hash and compute an HMAC-SHA512.
        """
        reversed_data = data[::-1]
        return hmac.new(self._key.encode(), reversed_data.encode(), hashlib.sha512).hexdigest()

    def _hash_layer3(self, data):
        """
        Third layer: Custom obfuscation.
        
        - Convert the SHA512 hex digest to bytes.
        - Generate a pattern from the secret key by taking its SHA256 digest.
        - XOR each byte of the data with the pattern (cycling through the pattern).
        - Finally, hash the resulting bytes with SHA256.
        """
        pattern = hashlib.sha256(self._key.encode()).digest()
        data_bytes = bytes.fromhex(data)
        obfuscated = bytearray()
        for i, b in enumerate(data_bytes):
            obfuscated.append(b ^ pattern[i % len(pattern)])
        return hashlib.sha256(obfuscated).hexdigest()

    def _sign_token(self, token):
        """
        Process the token through multiple hashing layers.
        """
        layer1 = self._hash_layer1(token)
        layer2 = self._hash_layer2(layer1)
        layer3 = self._hash_layer3(layer2)
        return layer3

    def get_signer(self):
        """
        Returns a reference to the hidden signing function.
        """
        return self._sign_token


def initFn(secret_key):
    """
    Initializes and returns an instance of the hidden module.
    """
    return HiddenModule(secret_key)
