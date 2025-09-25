; boot/bootsect.asm
bits 16

section .text   ;表示这是一个代码段，起始地址址为0x7C00
    org 0x7C00

start:
    ; 设置段寄存器
    cli         ;关闭中断
    xor ax, ax  ;此时ax为0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti         ;开启中断

    ; 显示启动信息
    mov si, msg_booting
    call print_string

    ; 加载第二阶段引导程序到内存
    mov ax, 0x8000      ; 加载段地址
    mov es, ax          ; 设置ES为加载段
    mov bx, 0x0000      ; 加载偏移地址
    mov ah, 0x02        ; 读取扇区功能
    mov al, 32          ; 读取32个扇区
    mov ch, 0           ; 磁道0
    mov cl, 2           ; 从第2个扇区开始
    mov dh, 0           ; 磁头0
    mov dl, 0x80        ; 第一个硬盘
    int 0x13            ; BIOS磁盘中断来加载扇区
    jc disk_error       ; 检查错误标志
    cmp al, 32          ; 检查实际读取的扇区数是否匹配
    jne disk_error

    ; 跳转到第二阶段
    jmp 0x8000:0x0000   ; 跳转到0x8000:0x0000处的代码段

; 打印字符串函数
print_string:
    pusha
print_char:
    lodsb               ; 读取si的一个字节到al
    cmp al, 0           ; 如果为0，则结束
    je print_done
    mov ah, 0x0E
    int 0x10            ; bios中断以显示字符
    jmp print_char      ; 循环打印字符
print_done:
    popa
    ret

disk_error:
    mov si, msg_error
    call print_string
    jmp $               ; 跳转到自己的指令地址，会死循环，程序被挂起

; 规定两个字符串
msg_booting db 'Booting...', 0x0D, 0x0A, 0
msg_error   db 'Disk Error!', 0

times 510-($-$$) db 0
dw 0xAA55