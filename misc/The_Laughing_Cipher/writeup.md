# The Laughing Cipher

## Challenge Overview

We are given three files:
- `encrypted_output.log`
- `message_for_batman.sh`
- `packets.pcapng`

From the message in `message_for_batman.sh`, we understand that the original `/home/gotham/logs/keys.log` file has been encrypted using a Bash script.

---

## Understanding the Encryption Mechanism

### Analyzing `message_for_batman.sh`

```bash
input_file="/home/gotham/logs/keys.log"
m1=$(md5sum "$filename")

while true; do
  sleep 1
  m2=$(md5sum "$filename")

  if [ "$m1" != "$m2" ] ; then
    m1=$m2
    main
  fi
done
```

- The script monitors `keys.log` for changes.
- If modified, it calls the `main` function.

### Encryption Logic

```bash
main() {
   while IFS= read -r line; do
        [ -z "$line" ] && continue
        
        read -r title value1 value2 <<< "$line"
        
        local key_length=${#title}
        local key=$(head -c "$key_length" /dev/urandom)
        
        local encrypted_title=$(encrypt_hex "$title" "$key")        
        local encrypted1=$(encrypt_hex "$value1" "$key")        
        local encrypted2=$(encrypt_hex "$value2" "$key")
        
        echo "$title $encrypted_title $encrypted1 $encrypted2" >> encrypted_output.log
        
    done < "$input_file"
}
```

- Reads each line from `keys.log`.
- Generates a random key based on the title's length.
- Encrypts fields using the `encrypt_hex` function.

```bash
encrypt_hex() {
    local input=$1
    local key=$2
    
    local input_hex=$(printf "%s" "$input" | xxd -p | tr -d '\n')
    local n=${#input_hex}
    
    local key_hex=$(printf "%s" "$key" | xxd -p | tr -d '\n')
    local temp="$key_hex"
    while [ ${#temp} -lt "$n" ]; do
        temp+="$key_hex"
    done
    local extended_key="${temp:0:$n}"

    local length=${#input_hex}
    local encrypted=""
    
    for ((i=0; i<length; i+=2)); do
        local byte1="${input_hex:$i:2}"
        local byte2="${extended_key:$i:2}"
        local dec1=$((16#$byte1))
        local dec2=$((16#$byte2))
        encrypted+="$hex_result"
    done
    
    echo "$encrypted"
}
```

- The encryption uses XOR with a key derived from `/dev/urandom`, similar to a One-Time Pad (OTP).
- Since the first field is known plaintext, we can recover the key and decrypt the rest.

---

## Decryption Script

```python
def extend_key(key, length):
    return (key * (length // len(key) + 1))[:length]

def decrypt(ciphertext, key):
    if not isinstance(ciphertext, bytes):
        ciphertext = ciphertext.encode('utf-8')
    
    extended_key = extend_key(key, len(ciphertext))
    
    message = bytes(m ^ k for m, k in zip(ciphertext, extended_key))
    return message.decode('utf-8')

def break_repeating_key_otp(known_plaintext, ciphertext):
    if not isinstance(known_plaintext, bytes):
        known_plaintext = known_plaintext.encode('utf-8')
    if not isinstance(ciphertext, bytes):
        ciphertext = bytes.fromhex(ciphertext)
    
    # Recover the key by XORing known plaintext with its ciphertext
    recovered_key = bytes(p ^ c for p, c in zip(known_plaintext, ciphertext))
    
    return recovered_key

def decrypt_messages(known_plaintext, encrypted_hex, messages):
    # Get the key using the known plaintext and its encrypted version
    key = break_repeating_key_otp(known_plaintext, encrypted_hex)
    
    # Decrypt all messages using the recovered key
    decrypted = []
    for message in messages:
        try:
            decrypted_msg = decrypt(bytes.fromhex(message), key)
            decrypted.append(decrypted_msg)
        except Exception as e:
            decrypted.append(f"Error decrypting message: {str(e)}")
    
    return key, decrypted

def main():
    # Read input file
    with open('encrypted_output.log', 'r') as file:
        lines = file.readlines()
    
    # Open output file
    with open('output.log', 'w') as outfile:
        # Process each line
        for line in lines:
            # Split the line into components
            parts = line.strip().split()
            if len(parts) >= 4:  # Ensure we have all required parts
                plaintext = parts[0]
                encrypted_hex = parts[1]
                messages = parts[2:]
                
                # Decrypt the messages
                key, decrypted_messages = decrypt_messages(plaintext, encrypted_hex, messages)
                print(f"Recovered key for {plaintext}: {key}")
                
                # Write results to output file in the format: plaintext message1 message2
                outfile.write(f"{plaintext} {decrypted_messages[0]} {decrypted_messages[1]}\n")

if __name__ == "__main__":
    main()
```

- Using the known plaintext, we extract the key.
- The key is then used to decrypt the remaining fields.

From the decrypted log, we obtain an **SSLKEYLOGFILE**:

```
SERVER_HANDSHAKE_TRAFFIC_SECRET bd79aa5d89c579fcc0b350f44d8cc1dd1b70384a88a3f6ce58993a442c4fbe2a 720232eed550daa42a1b7d6e6171c08ef8f6427c2ce99795d4141e2ebccb1fc3
EXPORTER_SECRET bd79aa5d89c579fcc0b350f44d8cc1dd1b70384a88a3f6ce58993a442c4fbe2a 06e3b12e66562e88c84197efc18ca868fb82684ba6b3282a96b59f72a3cefa19
SERVER_TRAFFIC_SECRET_0 bd79aa5d89c579fcc0b350f44d8cc1dd1b70384a88a3f6ce58993a442c4fbe2a 6725d124391fff5bce3aea98885219deebef15fc94c004da0adbcb6f5ade6c0b
CLIENT_HANDSHAKE_TRAFFIC_SECRET bd79aa5d89c579fcc0b350f44d8cc1dd1b70384a88a3f6ce58993a442c4fbe2a 9ed31dfecf3398c17173f377c3a3fbd7c20754ae0039b2dbbbfe11e1a3cd6aa9
CLIENT_TRAFFIC_SECRET_0 bd79aa5d89c579fcc0b350f44d8cc1dd1b70384a88a3f6ce58993a442c4fbe2a 6216515785896fe36deea19edd9255225dd9429af73be3fa85ec32f370ab961b
```

Using this file, we decrypted the HTTPS traffic in `packets.pcapng` using **Wireshark**.

## Bleichenbacher's Attack

A POST request was found to `/api/decrypt` on `https://verify.ctf.prgy.in/`.

DNS request reveals subdomain.
![dns](https://github.com/user-attachments/assets/93e0081a-258e-47e0-b0d0-5282a384cbe3)

We can find the endpoint and other information after decrypting the packet.
![packet](https://github.com/user-attachments/assets/201c344e-8fa9-48fa-81a5-5e4108581985)

The POST request included the following parameters:
```json
{
    "ciphertext": "09bc7f69e333dd9104b52286f1c5ff9aa4b2291c5cf3be7c38b2a98d0a68f02799851381812302ae69090bb07e922f107b67485bcbc0c3d724241618425df8162ccdca165b0a97c083c330b931a1d6f46215adab7c68295d66728e676c1d108b6a572e626cd293b0bbe2ef314f3909ef7f7b8ce7029aa03c71ccff146d4357d2", 
    "N": 90755263541684588974054530792483677455898840942739210225029812710552341527556120481051645722505814811089681839278418335307312601942090489438507544120211485428365857239966842965248967998078147498280578516594079083591608859724969009400595378977246756348937112658062561728758753393034769353279956927820641491047, 
    "E": 3
}
```

If we make the same request to the endpoint, we get this response
```json
{
    "valid": true,
    "message": "Valid PKCS1 padding"
}
```

If we try to send a different request in hex to the same endpoint, we get this response
```json
{
    "valid": true,
    "message": "Invalid prefix"
}
```

If we send a ciphertext which is not in hex format, we get this response 
```json
{
    "valid": false,
    "message": "Invalid request format"
}
```

This is indicative of **Bleichenbacher’s RSA Padding Oracle Attack**, which works when we have an oracle which lets you distinguish between a valid and an invalid *PKCS#1 v1.5* padded ciphertext. Using the endpoint as an oracle, we launched an attack to decrypt the ciphertext using the given N and e in the POST request.

### Bleichenbacher Attack Script

```python
import os
import rsa
import requests
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry
from collections import namedtuple

Interval = namedtuple("Interval", ["lower_bound", "upper_bound"])

retry_strategy = Retry(
    total=3,
    backoff_factor=1,
    status_forcelist=[500, 502, 503, 504]
)
adapter = HTTPAdapter(max_retries=retry_strategy)
session = requests.Session()
session.mount("https://", adapter)

modulus_size = 1024
k = modulus_size // 8

n = 90755263541684588974054530792483677455898840942739210225029812710552341527556120481051645722505814811089681839278418335307312601942090489438507544120211485428365857239966842965248967998078147498280578516594079083591608859724969009400595378977246756348937112658062561728758753393034769353279956927820641491047
e = 3

url = "https://verify.ctf.prgy.in/api/decrypt"

def floor(a, b):
    return a // b

def ceil(a, b):
    return a // b + (a % b > 0)

def integer_to_bytes(integer):
    k = integer.bit_length()

    bytes_length = k // 8 + (k % 8 > 0)

    bytes_obj = integer.to_bytes(bytes_length, byteorder="big")

    return bytes_obj

def generate_key(modulus_length):
    (pubkey, privkey) = rsa.newkeys(modulus_length, exponent=3)

    return (pubkey.n, pubkey.e), (privkey.n, privkey.d)

def encrypt_integer(public_key, m):
    (n, e) = public_key

    if m > n:
        raise ValueError("Message is too big for current RSA scheme!")

    return pow(m, e, n)

def encrypt_string(public_key, message):
    integer = int.from_bytes(message, byteorder="big")
    enc_integer = encrypt_integer(public_key, integer)
    enc_string = integer_to_bytes(enc_integer)

    return enc_string

def PKCS1_encode(message, total_bytes):
    if len(message) > total_bytes - 11:
        raise Exception("Message to big for encoding scheme!")

    pad_len = total_bytes - 3 - len(message)

    padding = b""
    while len(padding) < pad_len:
        needed_bytes = pad_len - len(padding)
        new_padding = os.urandom(needed_bytes + 5)
        new_padding = new_padding.replace(b"\x00", b"")
        padding = padding + new_padding[:needed_bytes]

    assert len(padding) == pad_len

    encoded = b"\x00\x02" + padding + b"\x00" + message

    return encoded


def PKCS1_decode(encoded):
    encoded = encoded[2:]
    idx = encoded.index(b"\x00")

    message = encoded[idx + 1 :]

    return message

def query_oracle(ciphertext) -> bool:
    global session
    
    try:
        headers = {"Content-Type": "application/json"}
        payload = {
            "ciphertext": ciphertext,
            "N": 90755263541684588974054530792483677455898840942739210225029812710552341527556120481051645722505814811089681839278418335307312601942090489438507544120211485428365857239966842965248967998078147498280578516594079083591608859724969009400595378977246756348937112658062561728758753393034769353279956927820641491047,
            "E": 3
        }
        
        response = session.post(
            url,
            headers=headers,
            json=payload,
            verify=True,
            timeout=10
        )
        
        response.raise_for_status()
        result = response.json()
        
        if not isinstance(result, dict) or "valid" not in result:
            raise ValueError("Invalid response format")
            
        return result["valid"]
        
    except Exception as e:
        print(f"Request failed: {str(e)}")
        raise

def find_smallest_s(lower_bound, c):
    s = lower_bound

    while True:
        attempt = (c * pow(s, e, n)) % n
        attempt = integer_to_bytes(attempt)

        if query_oracle(attempt.hex()):
            return s

        s += 1

def find_s_in_range(a, b, prev_s, B, c):
    ri = ceil(2 * (b * prev_s - 2 * B), n)

    while True:
        si_lower = ceil(2 * B + ri * n, b)
        si_upper = ceil(3 * B + ri * n, a)

        for si in range(si_lower, si_upper):
            attempt = (c * pow(si, e, n)) % n
            attempt = integer_to_bytes(attempt)

            if query_oracle(attempt.hex()):
                return si

        ri += 1

def safe_interval_insert(M_new, interval):
    for i, (a, b) in enumerate(M_new):
        if (b >= interval.lower_bound) and (a <= interval.upper_bound):
            lb = min(a, interval.lower_bound)
            ub = max(b, interval.upper_bound)

            M_new[i] = Interval(lb, ub)
            return M_new

    M_new.append(interval)
    return M_new

def update_intervals(M, s, B):
    M_new = []

    for a, b in M:
        r_lower = ceil(a * s - 3 * B + 1, n)
        r_upper = ceil(b * s - 2 * B, n)

        for r in range(r_lower, r_upper):
            lower_bound = max(a, ceil(2 * B + r * n, s))
            upper_bound = min(b, floor(3 * B - 1 + r * n, s))

            interval = Interval(lower_bound, upper_bound)
            M_new = safe_interval_insert(M_new, interval)

    M.clear()
    return M_new

def bleichenbacher(ciphertext):
    c = int.from_bytes(ciphertext, byteorder="big")

    B = 2 ** (8 * (k - 2))

    M = [Interval(2 * B, 3 * B - 1)]

    s = find_smallest_s(ceil(n, 3 * B), c)

    M = update_intervals(M, s, B)

    while True:
        if len(M) >= 2:
            s = find_smallest_s(s + 1, c)

        elif len(M) == 1:
            a, b = M[0]

            if a == b:
                return integer_to_bytes(a % n)

            s = find_s_in_range(a, b, s, B, c)

        M = update_intervals(M, s, B)

def main():
    ciphertext = bytes.fromhex("09bc7f69e333dd9104b52286f1c5ff9aa4b2291c5cf3be7c38b2a98d0a68f02799851381812302ae69090bb07e922f107b67485bcbc0c3d724241618425df8162ccdca165b0a97c083c330b931a1d6f46215adab7c68295d66728e676c1d108b6a572e626cd293b0bbe2ef314f3909ef7f7b8ce7029aa03c71ccff146d4357d2")
    decrypted = bleichenbacher(ciphertext)
    decrypted = PKCS1_decode(decrypted)

    print("decrypted: ", decrypted)

if __name__ == "__main__":
    main()
```

Running the attack revealed the flag:

```bash
o0psies_Ch4ng3D_my_m1nd
```

**Final Flag:**
```bash
p_ctf{o0psies_Ch4ng3D_my_m1nd}
```
