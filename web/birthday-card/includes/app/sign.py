import hmac
import hashlib

class HiddenModule:
    """
    A hidden module that signs tokens using a secret key.
    """
    def __init__(self, key):
        self._key = key

    def _sign_token(self, token):
        """
        Internal signing function (hidden).
        """
        return hmac.new(self._key.encode(), token.encode(), hashlib.sha256).hexdigest()

    def get_signer(self):
        """
        Returns a reference to the hidden signing function.
        """
        return self._sign_token


def initFn(secret_key):
    return HiddenModule(secret_key)



