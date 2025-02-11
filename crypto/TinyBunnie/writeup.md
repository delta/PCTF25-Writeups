# TINYBUNNIE - Crypto CTF Writeup

## Source Code Analysis

- From this part of the code, we can figure out that to validate the block, we have to provide a Proof of Work (PoW) such that the following condition is satisfied:

```python
if enc1 == enc2 and PoW1 != PoW2:
                valid_blocks.append(block)
                print("Good job validating the block!")
```

- This concept relates to equivalent keys in encryption schemes.
- From the source code, we can determine that the encryption used is the **TEA (Tiny Encryption Algorithm)**, which was used in Microsoft’s XBOX. The [Wikipedia page](https://en.wikipedia.org/wiki/Tiny_Encryption_Algorithm) has a decryption script in C.
- The challenge requires us to input a pair of equivalent keys one at a time to validate a block. We need to provide six such pairs to get the flag, causing an **'overflow' in the 'chain'.**
- The name **TINYBUNNIE** hints at Tiny Encryption and Andrew "bunnie" Huang, who reverse-engineered the XBOX.

### Key Splitting Mechanism in TEA

- Looking at the part where the key is split into four parts and each is XOR'ed with its counterpart:

```python
s += self.DELTA
m0 += ((m1 << 4) + K[0]) ^ (m1 + s) ^ ((m1 >> 5) + K[1])
m0 &= msk
m1 += ((m0 << 4) + K[2]) ^ (m0 + s) ^ ((m0 >> 5) + K[3])
m1 &= msk
```

- We can infer that the initial key is split into four parts: `k0, k1, k2, k3`. During each round, either `k0` and `k1` or `k2` and `k3` are XOR'ed together at a time.
- This means that we can change the **MSB (Most Significant Bit)** of both numbers being XOR'ed, and the final answer will remain unchanged.
- In each of the 32 rounds of the loop, there are XOR operations that involve `K[0]` and `K[1]` in one operation, and `K[2]` and `K[3]` in another.
- Due to XOR properties, we can **complement the MSB of `K[0]` and `K[1]`** and still get the same result. The same applies to `K[2]` and `K[3]`.
- This concept is called **Equivalent Keys in TEA Encryption**. Each key has three equivalent keys, reducing the **total key space of TEA encryption from 2¹²⁸ to 2¹²⁶**. More details can be found in this [paper](https://www.schneier.com/wp-content/uploads/2016/02/paper-key-schedule.pdf).

### Generating Equivalent Keys

- Generating equivalent keys is straightforward. Split the 16-byte key into four parts and XOR the **MSB of `k0` and `k1` or `k2` and `k3` one at a time and both together**, resulting in **three other keys**.

```python
def generate_equivalent_keys(hex_key):
    """
    Given a 128-bit TEA key as a hex string, generate its three equivalent keys.
    """
    MASK = 0x80000000  # The XOR mask used to generate equivalent keys

    # Convert hex string to four 32-bit integers
    key_bytes = bytes.fromhex(hex_key)
    key_parts = [int.from_bytes(key_bytes[i:i+4], 'big') for i in range(0, 16, 4)]

    # Generate the equivalent keys
    key1 = [key_parts[0], key_parts[1], key_parts[2] ^ MASK, key_parts[3] ^ MASK]
    key2 = [key_parts[0] ^ MASK, key_parts[1] ^ MASK, key_parts[2], key_parts[3]]
    key3 = [key_parts[0] ^ MASK, key_parts[1] ^ MASK, key_parts[2] ^ MASK, key_parts[3] ^ MASK]

    # Convert back to hex strings
    equivalent_keys = [
        ''.join(format(k, '08x') for k in key1),
        ''.join(format(k, '08x') for k in key2),
        ''.join(format(k, '08x') for k in key3)
    ]

    return equivalent_keys

# Example Usage
sample_key = "6f37aad7b063c799481334f3c59d83d0"
equivalent_keys = generate_equivalent_keys(sample_key)

print("Original Key:   ", sample_key)
for i, key in enumerate(equivalent_keys, 1):
    print(f"Equivalent Key {i}:", key)
```

### Exploiting Equivalent Keys to Solve the Challenge

- Once we can generate the equivalent keys, we can satisfy the condition given to validate the blocks in the challenge.
- We can have:
>C(4,2) = 4! / (2!(4-2)!) = 4! / (2!2!) = (4 × 3) / (2 × 1) = 6

- We can use **pwntools** to send these to the server and retrieve the flag:

```python
from pwn import remote
import os
import itertools
import sys

def generate_random_hex():
    return os.urandom(16).hex()

def generate_equivalent_keys(hex_key):
    MASK = 0x80000000
    key_bytes = bytes.fromhex(hex_key)
    key_parts = [int.from_bytes(key_bytes[i:i+4], 'big') for i in range(0, 16, 4)]
    key1 = [key_parts[0], key_parts[1], key_parts[2] ^ MASK, key_parts[3] ^ MASK]
    key2 = [key_parts[0] ^ MASK, key_parts[1] ^ MASK, key_parts[2], key_parts[3]]
    key3 = [key_parts[0] ^ MASK, key_parts[1] ^ MASK, key_parts[2] ^ MASK, key_parts[3] ^ MASK]
    return [hex_key, ''.join(format(k, '08x') for k in key1),
            ''.join(format(k, '08x') for k in key2), ''.join(format(k, '08x') for k in key3)]

host, port = 'localhost', 5000
io = remote(host, port)

for _ in range(6):
    io.recvuntil(b'Validate this block:')
    block = io.recvline().decode().strip()
    print(f"Validating block: {block}")
    base_key = generate_random_hex()
    equivalent_keys = generate_equivalent_keys(base_key)
    key_combinations = list(itertools.combinations(equivalent_keys, 2))
    for key1, key2 in key_combinations:
        io.sendlineafter(b"Enter first ProofOfWork  :", key1.encode())
        io.sendlineafter(b"Enter second ProofOfWork :", key2.encode())
        response = io.recvline().decode()
        print(response)
        response = io.recvline().decode()
        print(response)
        if "Good job" in response or "p_ctf{" in response:
            print(f"Flag found: {response.strip()}")
            io.close()
            sys.exit(0)
io.interactive()
```

### Retrieving the Flag

```shell
Good job validating the block!
Wait Noooooooooo: p_ctf{0Hh_No00_I5_T(-)i$_value_Ov3rFl0W!}
```