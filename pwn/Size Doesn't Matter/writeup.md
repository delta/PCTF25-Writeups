## Binary Analysis

Reversing the binary in Ghidra reveals the following flow:

```  
        00401000 48 be 00        MOV        RSI,msg                                          = "I'm small, aren't I? Nobody e
                 20 40 00 
                 00 00 00 00
        0040100a ba 3a 00        MOV        EDX,0x3a
                 00 00
        0040100f b8 01 00        MOV        EAX,0x1
                 00 00
        00401014 bf 01 00        MOV        EDI,0x1
                 00 00
        00401019 0f 05           SYSCALL
        0040101b 48 81 ec        SUB        RSP,0x1f4
                 f4 01 00 00
        00401022 48 89 e6        MOV        RSI,RSP
        00401025 ba f3 01        MOV        EDX,0x1f3
                 00 00
        0040102a b8 00 00        MOV        EAX,0x0
                 00 00
        0040102f bf 00 00        MOV        EDI,0x0
                 00 00
        00401034 0f 05           SYSCALL
        00401036 48 be 3a        MOV        RSI,msg2                                         = "I guess I'll just stay here, 
                 20 40 00 
                 00 00 00 00
        00401040 ba 3c 00        MOV        EDX,0x3c
                 00 00
        00401045 b8 01 00        MOV        EAX,0x1
                 00 00
        0040104a bf 01 00        MOV        EDI,0x1
                 00 00
        0040104f 0f 05           SYSCALL
        00401051 48 be b0        MOV        RSI,buffer                                     
                 20 40 00 
                 00 00 00 00
        0040105b ba 10 00        MOV        EDX,0x10
                 00 00
        00401060 b8 00 00        MOV        EAX,0x0
                 00 00
        00401065 bf 00 00        MOV        EDI,0x0
                 00 00
        0040106a 0f 05           SYSCALL
        0040106c 0f 05           SYSCALL
        0040106e 48 be 76        MOV        RSI,msg3                                         = "Guess I was right... I'm just
                 20 40 00 
                 00 00 00 00
        00401078 ba 39 00        MOV        EDX,0x39
                 00 00
        0040107d b8 01 00        MOV        EAX,0x1
                 00 00
        00401082 bf 01 00        MOV        EDI,0x1
                 00 00
        00401087 0f 05           SYSCALL
        00401089 48 81 c4        ADD        RSP,0x1f4
                 f4 01 00 00
        00401090 b8 3c 00        MOV        EAX,0x3c
                 00 00
        00401095 48 31 ff        XOR        RDI,RDI
        00401098 0f 05           SYSCALL
```

```
                             __bss_start                                     XREF[3]:     Entry Point(*), 
                             buffer                                                       entry:00401051(*), 
                                                                                          _elfSectionHeaders::000000d0(*)  
        004020b0                 ??         ??
        004020b1                 ??         ??
        004020b2                 ??         ??
        004020b3                 ??         ??
        004020b4                 ??         ??
        004020b5                 ??         ??
        004020b6                 ??         ??
        004020b7                 ??         ??
        004020b8                 ??         ??
        004020b9                 ??         ??
        004020ba                 ??         ??
        004020bb                 ??         ??
        004020bc                 ??         ??
        004020bd                 ??         ??
        004020be                 ??         ??
        004020bf                 ??         ??
        004020c0                 ??         ??
        004020c1                 ??         ??
        004020c2                 ??         ??
        004020c3                 ??         ??
        004020c4                 ??         ??
        004020c5                 ??         ??
        004020c6                 ??         ??
        004020c7                 ??         ??
```

1. **Write Syscall**: The program prints a message (`msg`) using the `write` syscall.
2. **First Read Syscall**: It reads **499 bytes** of input into the stack.
3. **Write Syscall**: Another message (`msg2`) is printed.
4. **Second Read Syscall**: It reads **16 bytes** of input into the `.bss` section.
5. **Write Syscall**: A concluding message (`msg3`) is printed.
6. **Exit**: The program terminates with a `syscall`.

---

### Observations

1. **No Buffer Overflow**: The binary limits the read size, preventing stack-based buffer overflow.
2. **Syscall Duplication**: The second read performs two consecutive `syscall` instructions.
3. **Register Control**: The `read` syscall returns the number of bytes read in the `rax` register. Since the maximum input size for the second `read` is 16 bytes, we can control `rax` with values up to 16.
4. **Useful Syscall**: One of the syscalls we can invoke with a controlled `rax` is `sigreturn` (15).

---

## `sigreturn`

`sigreturn` is a special syscall used during signal handling to restore the CPU's state from a signal frame stored on the stack. By crafting a fake signal frame, we can control all CPU registers and execute arbitrary syscalls.

When the `sigreturn` syscall is executed, it interprets the contents of the stack as a signal frame and restores the state of the registers to the values specified in the frame. This allows us to:

- Fully control the registers (`rax`, `rdi`, `rsi`, `rdx`, etc.).
- Prepare and execute a desired syscall (e.g., `execve`).

---

## Objective: Spawn a Shell

To spawn a shell, we need to call the `execve` syscall (`rax = 59`) with the following arguments:

| Register | Value           | Description       |
|----------|-----------------|-------------------|
| `rdi`    | `/bin/sh`       | Path to the shell |
| `rsi`    | `NULL`          | Arguments pointer |
| `rdx`    | `NULL`          | Environment pointer |

---

### Key Exploit Insight: Passing `/bin/sh` to `rdi`

One major challenge in spawning a shell is ensuring the string `/bin/sh` is available in memory at a known address, which is required to pass as the `rdi` argument to the `execve` syscall.

- **Stack Address Uncertainty**: Since the binary does not leak any stack address, we cannot reliably place `/bin/sh` on the stack for `execve`. 
- **`.bss` Section Advantage**: The second `read` syscall writes into the `.bss` section, a writable and non-randomized memory region. This is crucial as we can control the content and know its exact address.

### Why 15 Bytes?

- The second `read` syscall reads a maximum of **16 bytes** into the `.bss` section.  
- To control the `rax` register for the `sigreturn` syscall, we need to set it to **15** (`rax = number of bytes read`).  
- By sending `/bin/sh\x00AAAAAAA` (15 bytes including the null terminator), we ensure:
  1. The string `/bin/sh\x00AAAAAAA` is stored in `.bss` at a known location.
  2. `rax` is set to **15**, allowing us to invoke `sigreturn` immediately.

### Exploitation Strategy

1. Use the second `read` syscall to store `/bin/sh\x00AAAAAAA` in the `.bss` section.  
2. Use this address as the value for the `rdi` register during the `sigreturn` syscall.  
3. Combine it with a crafted signal frame to execute the `execve` syscall.


---

## Final Payload

1. First input:
   - Prepare a crafted signal frame for the `sigreturn` syscall.
2. Second input:
   - Pass `/bin/sh\x00AAAAAAA` to the `.bss` section to satisfy the `execve` syscall arguments.

For the complete payload, refer to the provided script: [solve.py](solve.py)

---
