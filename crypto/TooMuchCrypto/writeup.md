# TooMuchCrypto

The challenge is, at its core, a Preimage Attack on Round-Reduced BLAKE.

## SOURCE CODE ANALYSIS

- The given source code implements a round reduced blake 256 hash, and prints the initial, intermediate and the final state values into the ``output.py`` file.
- It also writes the values ``m[8]``, ``m[10]``, ``m[11]`` into the same file.
- The final hash of our flag is also written into the same file but we will later realize it's not necessary for finding the flag.

- Custom BLAKE-32 Hashing with State Manipulation:
    >Implements a modified BLAKE-32 hashing function, initializing and processing message blocks.
    >Stores intermediate state values (v0, v0_5, v1_5) during hashing for later use.
    >Extracts and saves certain message words (mm) and state values (v0510).

- RSA Encryption with Partial Prime Leakage:
    >Encrypts values using 2048-bit RSA with randomly generated strong primes.
    >Breaks primes into 16-bit chunks, shuffles them, and writes partial information (p_chunks, q_chunks) to output.py.
    >Stores the RSA ciphertext (c) and modulus (N) for later decryption attempts.

- Logging and Output Generation:
    >Encodes a secret flag (Flag) and computes its BLAKE-32 hash.
    >Writes intermediate state values, partial message words, RSA-encrypted values, and hash output to output.py for further cryptanalysis.
---
### **What is BLAKE?**
BLAKE is a cryptographic hash function that was a **finalist in the SHA-3 competition**. It is based on the **ChaCha** stream cipher and consists of:
- A **state vector** \( v \) of 16 words (512 bits)
- A **message block** \( m \) of 16 words (512 bits)
- A **round function** that iteratively updates the state using the **G function**

### **State Vector in BLAKE**
The state \( v \) consists of:
1. **Eight chaining variables** derived from the previous hash state
2. **Eight fixed constants** from the SHA-2 constants

The state is **mixed in rounds** using the **G function**, which applies:
- **XOR operations**
- **Rotations (ROTL)**
- **Additions modulo \( 2^{32} \)**

---

- The first step in the challenge is to recover the state values, which are written in ``output.py`` encrypted with RSA, which can be recovered by:

```python
import ast
from collections import deque
from Crypto.Util.number import inverse, long_to_bytes

def bfs_find_p_q(p_chunks, q_chunks, N, length):
    queue = deque([(0, [], [])])

    while queue:
        i, current_ps, current_qs = queue.popleft()

        if i == length:
            return current_ps, current_qs

        partial_N = N % 2 ** (16 * (i + 1))
        partial_p_poly = sum(
            [pi * (2 ** (16 * idx)) for idx, pi in enumerate(current_ps)]
        ) % (2 ** (16 * (i + 1)))
        partial_q_poly = sum(
            [qi * (2 ** (16 * idx)) for idx, qi in enumerate(current_qs)]
        ) % (2 ** (16 * (i + 1)))

        for pi in p_chunks:
            for qi in q_chunks:
                if (
                    (partial_p_poly + pi * (2 ** (16 * i)))
                    * (partial_q_poly + qi * (2 ** (16 * i)))
                ) % (2 ** (16 * (i + 1))) == partial_N:
                    queue.append((i + 1, current_ps + [pi], current_qs + [qi]))

    raise Exception("No valid solution found.")

def decrypt_rsa(N, c, p_chunks, q_chunks):
    LENGTH = len(p_chunks)

    # Find p and q using BFS
    found_ps, found_qs = bfs_find_p_q(p_chunks, q_chunks, N, LENGTH)

    # Reconstruct p and q
    p = sum([pi * (2 ** (16 * idx)) for idx, pi in enumerate(found_ps)])
    q = sum([qi * (2 ** (16 * idx)) for idx, qi in enumerate(found_qs)])

    # Ensure p * q matches N
    assert p * q == N

    # Decrypt the message
    e = 0x10001
    m = pow(c, inverse(e, (p - 1) * (q - 1)), p * q)
    plaintext = long_to_bytes(m)
    return plaintext.hex()

# Read the file and process
file_name = "../includes/output.py"
decrypted_results = []

with open(file_name, "r") as file:
    lines = file.readlines()

# Process 4 lines at a time
for i in range(0, len(lines), 4):
    try:
        # Parse the variables from each chunk
        N = int(lines[i].split('=')[1].strip())
        c = int(lines[i + 1].split('=')[1].strip())
        p_chunks = ast.literal_eval(lines[i + 2].split('=')[1].strip())
        q_chunks = ast.literal_eval(lines[i + 3].split('=')[1].strip())

        # Decrypt using the parsed values
        decr_m = decrypt_rsa(N, c, p_chunks, q_chunks)
        
        decr_m = f"0x{decr_m}"
        print(decr_m)
        # decr_m=int((decr_m))
        decrypted_results.append(decr_m)

    except Exception as e:
        print(f"Error processing chunk starting at line {i + 1}: {e}")

v0=[]
v1_5=[]
for i in range(len(decrypted_results)):
    if(i%2==0):
        v0.append(decrypted_results[i])
    else:
        v1_5.append(decrypted_results[i])

print(f"v0={v0}")
print(f"v1_5={v1_5}")


v0 = [int(x, 16) for x in v0]
v1_5 = [int(x, 16) for x in v1_5]

# Print in C array format
print(f"uint32_t v0[16] = {{{', '.join(f'0x{x:08x}' for x in v0)}}};")
print(f"uint32_t v1_5[16] = {{{', '.join(f'0x{x:08x}' for x in v1_5)}}};")
```

The function bfs_find_p_q() performs Breadth-First Search (BFS) to systematically reconstruct pp and qq by:

    Starting from the least significant 16-bit chunk.
    Iterating over all possible chunk combinations.
    Ensuring at each step that the partial reconstruction satisfies p×q≡Nmod  216(i+1)p×q≡Nmod216(i+1).
    Once all chunks are placed correctly, pp and qq are fully reconstructed.

Why Does BFS Work?

    Each step ensures the lower bits of pp and qq correctly multiply to the lower bits of NN, making incorrect guesses impossible.
    The modular constraint ensures that as we expand our reconstruction, every extension must remain valid.
    Since 16-bit chunks are small, BFS efficiently finds valid reconstructions without excessive computational cost.


****

## **Understanding the Preimage Attack**

### **What is a Preimage Attack?**
A **preimage attack** means that given a known intermediate state of the hash, we try to **reverse the computation** to recover the **original message \( m \)**.

Given partial knowledge of the **original message \( m \) and state values \( v0, v1.5, v0.5 \)**, the function **preimage_attack()** applies inverse transformations to compute missing values.

The key idea behind the attack:
1. **Undo the round transformations** by reversing the G function
2. **Solve for missing message words** by reversing XORs, rotations, and subtractions
3. **Reconstruct the original input message**

-  I found an implementation for the attack in this [repo](https://github.com/ajaycc17/blake-256). Credits to [ajaycc17](https://github.com/ajaycc17).

```C

#include "blake_header.h"

uint32_t *preimage_attack(uint32_t v0[], uint32_t v1_5[], uint32_t m8, uint32_t m10, uint32_t m11, uint32_t v10)
{
    uint32_t state0_5[16] = {0}, state1[16] = {0};
    static uint32_t pred_mess[16] = {0};

    state1[4] = ROTL(ROTL(v1_5[4], 7) ^ v1_5[8], 12) ^ (v1_5[8] - v1_5[12]);
    state1[5] = ROTL(ROTL(v1_5[5], 7) ^ v1_5[9], 12) ^ (v1_5[9] - v1_5[13]);
    state1[6] = ROTL(ROTL(v1_5[6], 7) ^ v1_5[10], 12) ^ (v1_5[10] - v1_5[14]);
    state1[7] = ROTL(ROTL(v1_5[7], 7) ^ v1_5[11], 12) ^ (v1_5[11] - v1_5[15]);

    state1[8] = v1_5[8] - v1_5[12] - (ROTL(v1_5[12], 8) ^ v1_5[0]);
    state1[9] = v1_5[9] - v1_5[13] - (ROTL(v1_5[13], 8) ^ v1_5[1]);
    state1[10] = v1_5[10] - v1_5[14] - (ROTL(v1_5[14], 8) ^ v1_5[2]);
    state1[11] = v1_5[11] - v1_5[15] - (ROTL(v1_5[15], 8) ^ v1_5[3]);

    state1[12] = ROTL((ROTL(v1_5[12], 8) ^ v1_5[0]), 16) ^ (v1_5[0] - (ROTL(v1_5[4], 7) ^ v1_5[8]) - m10);
    state1[13] = ROTL((ROTL(v1_5[13], 8) ^ v1_5[1]), 16) ^ (v1_5[1] - (ROTL(v1_5[5], 7) ^ v1_5[9]) - m8);

    state0_5[6] = ROTL(ROTL(state1[6], 7) ^ state1[11], 12) ^ (state1[11] - state1[12]);
    state0_5[7] = ROTL(ROTL(state1[7], 7) ^ state1[8], 12) ^ (state1[8] - state1[13]);

    pred_mess[4] = (ROTL(((ROTL((ROTL(state0_5[6], 7) ^ v10), 12) ^ v0[6]) - v0[10]), 16) ^ v0[14]) - v0[2] - v0[6];

    state1[1] = (ROTL(((ROTL((ROTL(v1_5[5], 7) ^ v1_5[9]), 12) ^ state1[5]) - state1[9]), 16) ^ state1[13]) - state1[5] - pred_mess[4];

    state0_5[14] = v10 - v0[10] - ROT((v0[14] ^ (v0[2] + v0[6] + pred_mess[4])), 16);

    state0_5[1] = state1[1] - (ROTL(state1[6], 7) ^ state1[11]) - m11 - state0_5[6] - m10;

    state0_5[11] = state1[11] - state1[12] - (ROTL(state1[12], 8) ^ state1[1]);

    state0_5[12] = ROTL(((ROTL((ROTL(state1[6], 7) ^ state1[11]), 12) ^ state0_5[6]) - state0_5[11]), 16) ^ (state0_5[1] + state0_5[6] + m10);

    state0_5[2] = (v10 - state0_5[14] - v0[10]) ^ ROTL(state0_5[14], 8);

    pred_mess[5] = (ROTL(((ROTL((ROTL(state0_5[6], 7) ^ v10), 12) ^ v0[6]) - v0[10]), 16) ^ v0[14]) + (ROTL(state0_5[6], 7) ^ v10) - state0_5[2];
    pred_mess[5] = -pred_mess[5];

    pred_mess[6] = (ROTL(((ROTL((ROTL(state0_5[7], 7) ^ state0_5[11]), 12) ^ v0[7]) - v0[11]), 16) ^ v0[15]) - v0[7] - v0[3];

    state1[15] = ROTL((ROTL(v1_5[15], 8) ^ v1_5[3]), 16) ^ (v1_5[3] - (ROTL(v1_5[7], 7) ^ v1_5[11]) - pred_mess[6]);

    state0_5[15] = state0_5[11] - v0[11] - ROT((v0[15] ^ (v0[3] + v0[7] + pred_mess[6])), 16);

    state0_5[5] = ROTL((ROTL(state1[5], 7) ^ state1[10]), 12) ^ (state1[10] - state1[15]);

    state1[0] = (state1[10] - state1[15] - v10) ^ ROTL(state1[15], 8);

    pred_mess[9] = (ROTL(((ROTL((ROTL(state1[5], 7) ^ state1[10]), 12) ^ state0_5[5]) - v10), 16) ^ state0_5[15]) + (ROTL(state1[5], 7) ^ state1[10]) - state1[0];
    pred_mess[9] = -pred_mess[9];

    pred_mess[14] = (ROTL(((ROTL((ROTL(v1_5[4], 7) ^ v1_5[8]), 12) ^ state1[4]) - state1[8]), 16) ^ state1[12]) - state1[0] - state1[4];

    state0_5[3] = (state0_5[11] - state0_5[15] - v0[11]) ^ ROTL(state0_5[15], 8);

    pred_mess[7] = (ROTL(((ROTL((ROTL(state0_5[7], 7) ^ state0_5[11]), 12) ^ v0[7]) - v0[11]), 16) ^ v0[15]) + (ROTL(state0_5[7], 7) ^ state0_5[11]) - state0_5[3];
    pred_mess[7] = -pred_mess[7];

    state0_5[0] = (ROTL(((ROTL((ROTL(state1[5], 7) ^ state1[10]), 12) ^ state0_5[5]) - v10), 16) ^ state0_5[15]) - m8 - state0_5[5];

    state0_5[8] = v0[8] + state0_5[12] + (ROTL(state0_5[12], 8) ^ state0_5[0]);

    pred_mess[0] = (ROTL((ROTL(state0_5[12], 8) ^ state0_5[0]), 16) ^ v0[12]) - v0[4] - v0[0];

    state1[2] = (state1[8] - state1[13] - state0_5[8]) ^ ROTL(state1[13], 8);

    state1[14] = (state1[2] + pred_mess[9] + state1[6]) ^ ROTL(((ROTL((ROTL(v1_5[6], 7) ^ v1_5[10]), 12) ^ state1[6]) - state1[10]), 16);

    pred_mess[15] = (ROTL(((ROTL((ROTL(v1_5[6], 7) ^ v1_5[10]), 12) ^ state1[6]) - state1[10]), 16) ^ state1[14]) + (ROTL(v1_5[6], 7) ^ v1_5[10]) - v1_5[2];
    pred_mess[15] = -pred_mess[15];

    state0_5[4] = ROT((ROT((v0[4] ^ (state0_5[8] - state0_5[12])), 12) ^ state0_5[8]), 7);

    pred_mess[1] = (ROTL(((ROTL((ROTL(state0_5[4], 7) ^ state0_5[8]), 12) ^ v0[4]) - v0[8]), 16) ^ v0[12]) + (ROTL(state0_5[4], 7) ^ state0_5[8]) - state0_5[0];
    pred_mess[1] = -pred_mess[1];

    state0_5[9] = state1[9] - state1[14] - ROT((state0_5[14] ^ (state0_5[3] + state0_5[4] + pred_mess[14])), 16);

    state1[3] = (ROTL(((ROTL((ROTL(state1[4], 7) ^ state1[9]), 12) ^ state0_5[4]) - state0_5[9]), 16) ^ state0_5[14]) + (ROTL(state1[4], 7) ^ state1[9]) + pred_mess[15];

    pred_mess[13] = (ROTL(((ROTL((ROTL(v1_5[7], 7) ^ v1_5[11]), 12) ^ state1[7]) - state1[11]), 16) ^ state1[15]) - state1[3] - state1[7];

    pred_mess[2] = (ROTL(((ROTL((ROTL(state0_5[5], 7) ^ state0_5[9]), 12) ^ v0[5]) - v0[9]), 16) ^ v0[13]) - v0[1] - v0[5];

    pred_mess[3] = (ROTL(((ROTL((ROTL(state0_5[5], 7) ^ state0_5[9]), 12) ^ v0[5]) - v0[9]), 16) ^ v0[13]) + (ROTL(state0_5[5], 7) ^ state0_5[9]) - state0_5[1];
    pred_mess[3] = -pred_mess[3];

    state0_5[13] = ROTL((ROTL(state1[13], 8) ^ state1[2]), 16) ^ (state1[2] - (ROTL(state1[7], 7) ^ state1[8]) - pred_mess[13]);

    pred_mess[12] = (ROTL(((ROTL((ROTL(state1[7], 7) ^ state1[8]), 12) ^ state0_5[7]) - state0_5[8]), 16) ^ state0_5[13]) - state0_5[2] - state0_5[7];

    return pred_mess;
}

int main()
{
    uint32_t m[16] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x445f7730, 0x00000000, 0x65683321, 0x7d800000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t v0_5[16] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xbbcca2bf, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t v0[16] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19, 0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344, 0xa409394a, 0x299f30b8, 0x082efa98, 0xec4e6c89};
    uint32_t v1_5[16] = {0xbb3809b6, 0x3eb21951, 0x3473e1d7, 0x8eee1830, 0xab8148af, 0x51319e79, 0x5cd72840, 0xa67d0f4f, 0xab4bc747, 0xb116e7fe, 0x0efd1976, 0x1f2b9e39, 0xeb56f6bc, 0xc29b53ed, 0xdf355f55, 0x1ada623d};

    uint32_t *pred_m;
    pred_m = preimage_attack(v0, v1_5, m[8], m[10], m[11], v0_5[10]);

    for (int i = 0; i < 16; i++)
    {
        if (*(pred_m + i)==0x00000000){
            *(pred_m + i) = m[i];
        }
        printf("(%d): %08x != %08x\n",i, m[i], *(pred_m + i));

    printf("m = [");
    for (int i = 0; i < 16; i++) {
        printf("0x%08x", m[i]);
        if (i < 15) {
            printf(", ");
            if ((i + 1) % 4 == 0) 
                printf("\n     ");
        }
    }
    printf("];\n");
    
    for (int i = 0; i < 16; i++)
        {
            printf("0x%08x\n", *(pred_m + i));
        }
     printf("m = [");
    for (int i = 0; i < 16; i++) {
        printf("0x%08x", pred_m[i]);
        if (i < 15) {
            printf(", ");
            if ((i + 1) % 4 == 0) 
                printf("\n     ");
        }
    }
    printf("];\n");
    return 0;
}
}
```

### **Function: `preimage_attack()`**
```c
uint32_t *preimage_attack(uint32_t v0[], uint32_t v1_5[], uint32_t m8, uint32_t m10, uint32_t m11, uint32_t v10)
```
This function tries to **recover missing message words** using known state values.

### **Step 1: Initialize State Arrays**
```c
uint32_t state0_5[16] = {0}, state1[16] = {0};
static uint32_t pred_mess[16] = {0};
```
- `state0_5`: Stores intermediate state values.
- `state1`: Stores another intermediate state (one step ahead of `state0_5`).
- `pred_mess`: Stores predicted message values.

### **Step 2: Compute Intermediate State `state1`**
```c
state1[4] = ROTL(ROTL(v1_5[4], 7) ^ v1_5[8], 12) ^ (v1_5[8] - v1_5[12]);
state1[5] = ROTL(ROTL(v1_5[5], 7) ^ v1_5[9], 12) ^ (v1_5[9] - v1_5[13]);
```
- Applies BLAKE’s **G function** to compute `state1` values.
- **ROTL (rotate left)** and **XOR** ensure diffusion.
- Subtractions recover unknown values in the state.

### **Step 3: Recover Some Message Words**
```c
pred_mess[4] = (ROTL(((ROTL((ROTL(state0_5[6], 7) ^ v10), 12) ^ v0[6]) - v0[10]), 16) ^ v0[14]) - v0[2] - v0[6];
```
- Uses **inverse rotations and XORs** to recover message word \( m_4 \).
- Adjusts by subtracting known values from \( v0 \).

### **Step 4: Compute Additional State Values**
```c
state0_5[6] = ROTL(ROTL(state1[6], 7) ^ state1[11], 12) ^ (state1[11] - state1[12]);
```
- More state values are computed using **XOR and ROTL operations**.

### **Step 5: Predict More Message Words**
```c
pred_mess[5] = (ROTL(((ROTL((ROTL(state0_5[6], 7) ^ v10), 12) ^ v0[6]) - v0[10]), 16) ^ v0[14]) + (ROTL(state0_5[6], 7) ^ v10) - state0_5[2];
pred_mess[5] = -pred_mess[5];
```
- **Negates** the result to adjust the prediction.
- Uses **inverse transformations** of the G function.


### **Main Function**
```c
int main()
{
    uint32_t m[16] = { ... };  // Some known message words
    uint32_t v0_5[16] = { ... }; // Partial state information
    uint32_t v0[16] = { ... };  // Initial state values
    uint32_t v1_5[16] = { ... };  // Another intermediate state
```
- Initializes known **state** and **message** values.

```c
    uint32_t *pred_m;
    pred_m = preimage_attack(v0, v1_5, m[8], m[10], m[11], v0_5[10]);
```
- Calls `preimage_attack()` to recover missing **message bits**.

```c
    for (int i = 0; i < 16; i++)
    {
        if (*(pred_m + i)==0x00000000){
            *(pred_m + i) = m[i];
        }
        printf("(%d): %08x != %08x\n",i, m[i], *(pred_m + i));
    }
```
- **Compares the original message** \( m \) with **the predicted message**.
- If a value is missing, it is replaced with the known message value.

---

## **Summary of the Attack**
- **Goal:** Given partial state values and message bits, recover the **original message \( m \)**.
- **Approach:** Reverse the mixing operations (ROTL, XOR, subtraction) applied in BLAKE.
- **Key Observations:**
  - The attack exploits how the **G function** combines message and state values.
  - We solve for unknown message bits by **undoing rotations and XORs**.
  - The attack **bypasses the full complexity of BLAKE** by reducing the number of rounds.

---


- The final step in getting the flag is to transform the message array into readable characters

>Converts an array of 32-bit integers into a byte sequence.
Iterates through each 32-bit word in m.
Converts each word into 4 bytes using big-endian format.
Returns the byte-encoded message.

```python
def m_to_bytes(m):

    message = b""
    for word in m:
        message += word.to_bytes(4, "big")  # Convert 32-bit word to 4 bytes (big-endian)
    return message

def remove_padding(message):

    padding_length = message[-1]  # Get the padding length from the last byte
    if padding_length > len(message):
        raise ValueError("Invalid padding length.")
    return message[:-padding_length]  # Remove the padding bytes

def main():
    
    m = [0x705f6374, 0x667b4d72, 0x5f41756d, 0x34733530, 
     0x4e5f7430, 0x6c345f74, 0x6869735f, 0x77307531, 
     0x445f7730, 0x726b5f68, 0x65683321, 0x7d800000, 
     0x00000000, 0x00000001, 0x00000000, 0x00000168];

    # Step 1: Convert m[] to bytes
    reconstructed_message = m_to_bytes(m)
    print("Reconstructed message (with padding):", reconstructed_message)

    # Step 2: Remove padding
    try:
        original_message = remove_padding(reconstructed_message)
        print("Original message (without padding):", original_message.decode())
    except ValueError as e:
        print("Error removing padding:", e)

if __name__ == "__main__":
    main()
```
This returns the flag as :


```bash
python3 solvef.py 
Reconstructed message (with padding): b'P_CTF{Mr_AumA550n_t0l4_t4is_w041d_w0rk_heh3}\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01`'
```

