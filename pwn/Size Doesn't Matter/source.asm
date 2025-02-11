section .data
        msg db "I'm small, aren't I? Nobody expects me to do anything...", 10, 0
        msg_len equ $ - msg
        msg2 db "I guess I'll just stay here, too small to matter. Figures.", 10, 0
        msg2_len equ $ - msg2
        msg3 db "Guess I was right... I'm just too small for you to care.", 0
        msg3_len equ $ - msg3

section .bss
        buffer resb 24

section .text
        global _start

_start:

        mov rsi, msg
        mov rdx, msg_len
        mov rax, 1
        mov rdi, 1
        syscall

        sub rsp, 500  

        mov rsi, rsp       
        mov rdx, 499      
        mov rax, 0        
        mov rdi, 0          
        syscall

        mov rsi, msg2
        mov rdx, msg2_len
        mov rax, 1
        mov rdi, 1
        syscall

        mov rsi, buffer
        mov rdx, 16
        mov rax, 0
        mov rdi, 0
        syscall

        syscall

        mov rsi, msg3
        mov rdx, msg3_len
        mov rax, 1
        mov rdi, 1
        syscall

        add rsp, 500

        mov rax, 60        
        xor rdi, rdi  
        syscall
