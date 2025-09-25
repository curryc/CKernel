; boot/bootstage2.asm
bits 16

section .text
    org 0x8000

start:
    ; 设置堆栈
    cli
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFF
    sti

    ; 显示信息
    mov si, msg_protected_mode
    call print_string

    ; 设置GDT并进入保护模式
    lgdt [gdt_descriptor]
    
    ; 启用保护模式 (设置PE位)
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; 跳转到32位代码段
    jmp 0x08:protected_mode_start

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

msg_protected_mode db 'Entering Protected Mode...', 0x0D, 0x0A, 0

; GDT表
gdt_start:
    dq 0x0 ; 空描述符

gdt_code:
    dw 0xFFFF    ; 限制 (低16位)
    dw 0x0000    ; 基址 (低16位)
    db 0x00      ; 基址 (中8位)
    db 10011010b ; 访问字节 (代码段)
    db 11001111b ; 粒度 (4KB粒度, 32位)
    db 0x00      ; 基址 (高8位)

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b ; 访问字节 (数据段)
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; 选择子
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

bits 32
protected_mode_start:
    ; 设置数据段寄存器
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; 设置堆栈
    mov esp, 0x90000

    ; 显示信息
    mov esi, msg_32bit
    call print_string32

    ; 检测是否支持长模式
    call check_long_mode
    test eax, eax
    jz no_long_mode

    ; 设置长模式GDT并进入长模式
    lgdt [gdt64.descriptor]
    
    ; 启用PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; 设置页表 (临时4GB映射)
    mov eax, paging_page_table
    mov cr3, eax
    
    ; 启用长模式 (EFER.LME)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; 启用分页 (进入长模式)
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    ; 跳转到64位代码
    jmp 0x08:long_mode_start

no_long_mode:
    mov esi, msg_no_long_mode
    call print_string32
    jmp $

; 32位字符串打印函数
print_string32:
    pusha
    mov ebx, 0xB8000
    mov ecx, 0
.loop:
    mov al, [esi + ecx]
    cmp al, 0
    je .done
    mov ah, 0x0F
    mov [ebx], ax
    add ebx, 2
    inc ecx
    jmp .loop
.done:
    popa
    ret

; 检查长模式支持
check_long_mode:
    pushfd
    pushfd
    xor dword [esp], 0x200000 ; 翻转ID位
    popfd
    pushfd
    pop eax
    xor eax, [esp]
    popfd
    and eax, 0x200000
    jz .no_cpuid

    ; 检查CPUID功能1
    mov eax, 1
    cpuid
    test edx, 1 << 3 ; 检查是否支持CPUID
    jz .no_long_mode

    ; 检查扩展功能
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29 ; 检查是否支持长模式
    jz .no_long_mode

    mov eax, 1
    ret

.no_cpuid:
.no_long_mode:
    xor eax, eax
    ret

msg_32bit db 'In Protected Mode (32-bit)', 0
msg_no_long_mode db 'Long mode not supported!', 0

; 页表设置 (恒等映射前1GB)
ALIGN 4096
paging_page_table:
    ; PML4表 (第一个条目)
    dd paging_pdpt + 0x3 ; 读/写, 存在
    times 511 dd 0

paging_pdpt:
    ; PDPT表 (第一个条目)
    dd paging_pd + 0x3 ; 读/写, 存在
    times 511 dd 0

paging_pd:
    ; PD表 (64个条目, 每个映射16MB)
    %assign i 0
    %rep 64
        dd (i << 22) | 0x83 ; 读/写, 存在, 4MB页面
        %assign i i+1
    %endrep
    times (512-64) dd 0


; 64位GDT
gdt64:
    dq 0 ; 空描述符

.code:
    dq 0x00209A0000000000 ; 代码段 (64位)
.data:
    dq 0x0000920000000000 ; 数据段

.descriptor:
    dw .descriptor - gdt64 - 1
    dd gdt64

CODE64_SEG equ .code - gdt64
DATA64_SEG equ .data - gdt64

bits 64
long_mode_start:
    ; 设置64位数据段
    mov ax, DATA64_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; 设置新的堆栈
    mov rsp, 0x100000

    ; 清屏
    mov edi, 0xB8000
    mov ecx, 500
    mov eax, 0x00000000
    rep stosd

    ; 显示信息
    mov rsi, msg_64bit
    call print_string64

    ; 调用内核主函数
    ; 我们不能使用extern，因为这是二进制输出格式
    ; 相反，我们直接跳转到内核入口点（由链接器放置在0x100000）
    call 0x100000

    ; 如果kernel_main返回，则无限循环
.end:
    hlt
    jmp .end

; 64位字符串打印函数
print_string64:
    push rsi
    mov rdi, 0xB8000
.loop:
    mov al, [rsi]
    cmp al, 0
    je .done
    mov [rdi], al
    mov byte [rdi+1], 0x0F
    add rdi, 2
    inc rsi
    jmp .loop
.done:
    pop rsi
    ret

msg_64bit db 'In Long Mode (64-bit) - Kernel Starting', 0

; 确保第二阶段引导程序占用32个扇区(16384字节)
times 32*512-($-$$) db 0