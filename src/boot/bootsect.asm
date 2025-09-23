; boot/bootsect.asm  ——  512 字节 BIOS 引导扇区
bits 16
org 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov ah, 0x0E
    mov al, 'B'
    int 0x10

hang:
    hlt
    jmp hang

times 510-($-$$) db 0
dw 0xAA55