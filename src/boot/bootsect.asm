; boot/bootsect.asm
bits 16

section .text
    org 0x7C00

start:
    ; 设置段寄存器
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; 显示启动信息
    mov si, msg_booting
    call print_string

    ; 加载第二阶段引导程序到内存
    mov bx, 0x8000      ; 加载地址
    mov dh, 2           ; 扇区数
    call load_disk

    ; 跳转到第二阶段
    jmp 0x0000:0x8000

; 打印字符串函数
print_string:
    pusha
print_char:
    lodsb
    cmp al, 0
    je print_done
    mov ah, 0x0E
    int 0x10
    jmp print_char
print_done:
    popa
    ret

; 磁盘加载函数
load_disk:
    pusha
    mov ah, 0x02        ; 读取扇区
    mov al, dh          ; 扇区数
    mov ch, 0           ; 磁道0
    mov cl, 2           ; 扇区2 (从第2个扇区开始)
    mov dh, 0           ; 磁头0
    mov dl, 0x80        ; 第一个硬盘
    int 0x13            ; BIOS磁盘中断
    jc disk_error
    popa
    ret

disk_error:
    mov si, msg_error
    call print_string
    jmp $

msg_booting db 'Booting...', 0x0D, 0x0A, 0
msg_error   db 'Disk Error!', 0

times 510-($-$$) db 0
dw 0xAA55