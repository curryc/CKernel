/*
* aarch64 启动文件
*/
.section ".text.boot"  
.global _start  // 定义全局入口点

_start:
    // 设置栈指针
    ldr     x0, =_stack_top
    mov     sp, x0

    // 清除 BSS 段
    ldr     x0, =_bss_start
    ldr     x1, =_bss_end
    sub     x1, x1, x0
    bl      memset

    // 调用内核主函数
    bl      kernel_main

    // 如果内核返回，进入死循环
halt:
    wfe
    b       halt

// 简单的 memset 实现
memset:
    cmp     x1, #0
    beq     memset_done
    mov     x2, #0
memset_loop:
    strb    w2, [x0], #1
    sub     x1, x1, #1
    cmp     x1, #0
    b.ne    memset_loop
memset_done:
    ret