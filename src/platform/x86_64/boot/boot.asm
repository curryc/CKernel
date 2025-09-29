; boot/boot.asm
; 如果有GRUB支持，只需要此文件一个即可，无需bootsect和bootstage2
; 此项目并未使用此文件
bits 32

section .multiboot
align 8

; Multiboot2 头
dd 0xE85250D6                ; 魔数
dd 0                         ; 架构 (0 = i386)
dd multiboot_end - multiboot_start ; 头部长度
; 校验和
dd 0x100000000 - (0xE85250D6 + 0 + (multiboot_end - multiboot_start))

multiboot_start:
; 信息请求结束标记
dw 0    ; type
dw 0    ; flags
dd 8    ; size
multiboot_end:

section .text
global _start
extern kernel_main

_start:
    ; 设置堆栈指针
    mov esp, 0x90000
    
    ; 清屏
    mov edi, 0xB8000
    mov ecx, 500
    mov eax, 0x00000000
    rep stosd
    
    ; 显示 'K' 表示进入内核
    mov edi, 0xB8000
    mov al, 'K'
    mov ah, 0x0F
    mov [edi], ax
    
    ; 调用C主函数
    push 0                      ; multiboot info指针
    call kernel_main
    add esp, 4                  ; 清理堆栈参数

    ; 如果kernel_main返回，则进入无限循环
.end:
    hlt
    jmp .end