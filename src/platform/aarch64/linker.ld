ENTRY(_start)  // 定义入口点

SECTIONS
{
    . = 0x80000;  // 定义内核加载地址

    .text : ALIGN(16)
    {
        *(.text.boot)  // 确保 boot.S 的代码段在最前面
        *(.text .text.*)
    }

    .rodata : ALIGN(16)
    {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(16)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(16)
    {
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        _bss_end = .;
    }

    _stack_bottom = .;
    . = . + 0x10000;  // 分配 64KB 的栈空间
    _stack_top = .;
}