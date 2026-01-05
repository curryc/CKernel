/* linker.ld  */
/* 指定输出格式 */
OUTPUT_FORMAT(
    "elf64-x86-64",
    "elf64-x86-64",
    "elf64-x86-64"
)
/* 指定输出架构 */
OUTPUT_ARCH(i386:x86-64)
/* 设置入口点 */
ENTRY(_start)

/* 1. 内核虚拟基址 */
KERNEL_VIRT_BASE = 0xffffffff80000000;
/* 2. 物理加载地址（bootloader 仍放在 1 MB） */
KERNEL_PHYS_BASE = 1M;
__executable_start = KERNEL_PHYS_BASE;

SECTIONS {
    /* 设置可执行文件起始地址 */
    PROVIDE (__executable_start = SEGMENT_START("text-segment", 1M));
    /* 设置当前地址计数器到文本段起始位置 */
    . = SEGMENT_START("text-segment", 1M);
    
    /* 引导代码段，包含引导过程所需的文本、数据和BSS段 */
    .boot           : { *(.text.boot) *(.data.boot) *(.bss.boot) }
    
    /* 程序解释器段，通常用于动态链接 */
    .interp         : { *(.interp) }
    
    /* GNU构建ID注释段 */
    .note.gnu.build-id  : { *(.note.gnu.build-id) }
    
    /* 符号哈希表段 */
    .hash           : { *(.hash) }
    
    /* GNU风格的符号哈希表段 */
    .gnu.hash       : { *(.gnu.hash) }
    
    /* 动态链接符号表段 */
    .dynsym         : { *(.dynsym) }
    
    /* 动态链接字符串表段 */
    .dynstr         : { *(.dynstr) }
    
    /* 符号版本信息段 */
    .gnu.version    : { *(.gnu.version) }
    
    /* 符号版本定义段 */
    .gnu.version_d  : { *(.gnu.version_d) }
    
    /* 符号版本需求段 */
    .gnu.version_r  : { *(.gnu.version_r) }
    
    /* 动态重定位表段 */
    .rela.dyn       : {
        *(.rela.init)
        *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
        *(.rela.fini)
        *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
        *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
        *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
        *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
        *(.rela.ctors)
        *(.rela.dtors)
        *(.rela.got)
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
        *(.rela.ldata .rela.ldata.* .rela.gnu.linkonce.l.*)
        *(.rela.lbss .rela.lbss.* .rela.gnu.linkonce.lb.*)
        *(.rela.lrodata .rela.lrodata.* .rela.gnu.linkonce.lr.*)
        *(.rela.ifunc)
    }
    
    /* 过程链接表重定位段 */
    .rela.plt       : {
        *(.rela.plt)
        PROVIDE_HIDDEN (__rela_iplt_start = .);
        *(.rela.iplt)
        PROVIDE_HIDDEN (__rela_iplt_end = .);
    }
    
    /* 相对重定位段 */
    .relr.dyn : { *(.relr.dyn) }
    
    /* 按页面大小对齐 */
    . = ALIGN(CONSTANT (MAXPAGESIZE));
    
    /* 初始化代码段 */
    .init           : {
        KEEP (*(SORT_NONE(.init)))
    }
    
    /* 过程链接表段 */
    .plt            : { *(.plt) *(.iplt) }
    
    /* GOT过程链接表段 */
    .plt.got        : { *(.plt.got) }
    
    /* 安全过程链接表段 */
    .plt.sec        : { *(.plt.sec) }
    
    /* 程序代码段 */
    .text           : {
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
        *(.text.exit .text.exit.*)
        *(.text.startup .text.startup.*)
        *(.text.hot .text.hot.*)
        *(SORT(.text.sorted.*))
        *(.text .stub .text.* .gnu.linkonce.t.*)
        /* .gnu.warning sections are handled specially by elf.em.  */
        *(.gnu.warning)
    }
    
    /* 终止代码段 */
    .fini           : {
        KEEP (*(SORT_NONE(.fini)))
    }
    
    /* 提供文本段结束符号 */
    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
    
    /* 按页面大小对齐 */
    . = ALIGN(CONSTANT (MAXPAGESIZE));
    
    /* 调整只读数据段地址 */
    . = SEGMENT_START("rodata-segment", ALIGN(CONSTANT (MAXPAGESIZE)) + (. & (CONSTANT (MAXPAGESIZE) - 1)));
    
    /* 只读数据段 */
    .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
    
    /* 只读数据段1 */
    .rodata1        : { *(.rodata1) }
    
    /* 异常帧头段 */
    .eh_frame_hdr   : { *(.eh_frame_hdr) *(.eh_frame_entry .eh_frame_entry.*) }
    
    /* 异常帧段（只读） */
    .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) *(.eh_frame.*) }
    
    /* GCC异常表（只读） */
    .gcc_except_table   : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }
    
    /* GNU异常表（只读） */
    .gnu_extab   : ONLY_IF_RO { *(.gnu_extab*) }
    
    /* 异常范围段，由Sun/Oracle C++编译器生成（只读） */
    .exception_ranges   : ONLY_IF_RO { *(.exception_ranges*) }
    
    /* 调整数据段地址 */
    . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
    
    /* 异常处理相关段（读写） */
    .eh_frame       : ONLY_IF_RW { KEEP (*(.eh_frame)) *(.eh_frame.*) }
    .gnu_extab      : ONLY_IF_RW { *(.gnu_extab) }
    .gcc_except_table   : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }
    .exception_ranges   : ONLY_IF_RW { *(.exception_ranges*) }
    
    /* 线程本地存储数据段 */
    .tdata	  : {
        PROVIDE_HIDDEN (__tdata_start = .);
        *(.tdata .tdata.* .gnu.linkonce.td.*)
    }
    
    /* 线程本地存储BSS段 */
    .tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
    
    /* 预初始化数组段 */
    .preinit_array    : {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    }
    
    /* 初始化函数数组段 */
    .init_array    : {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
        PROVIDE_HIDDEN (__init_array_end = .);
    }
    
    /* 终止函数数组段 */
    .fini_array    : {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
    }
    
    /* 构造函数段 */
    .ctors          : {
        /* gcc uses crtbegin.o to find the start of
        the constructors, so we make sure it is
        first.  Because this is a wildcard, it
        doesn't matter if the user does not
        actually link against crtbegin.o; the
        linker won't look for a file to match a
        wildcard.  The wildcard also means that it
        doesn't matter which directory crtbegin.o
        is in.  */
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        /* We don't want to include the .ctor section from
        the crtend.o file until after the sorted ctors.
        The .ctor section from the crtend file contains the
        end of ctors marker and it must be last */
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    }
    
    /* 析构函数段 */
    .dtors          : {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
    }
    
    /* Java类引用段 */
    .jcr            : { KEEP (*(.jcr)) }
    
    /* 只读数据段（相对地址） */
    .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*) }
    
    /* 动态链接段 */
    .dynamic        : { *(.dynamic) }
    
    /* 全局偏移表段 */
    .got            : { *(.got) *(.igot) }
    
    /* 数据段只读结束标记 */
    . = DATA_SEGMENT_RELRO_END (SIZEOF (.got.plt) >= 24 ? 24 : 0, .);
    
    /* 全局偏移表过程链接段 */
    .got.plt        : { *(.got.plt) *(.igot.plt) }
    
    /* 数据段 */
    .data           : {
        *(.data .data.* .gnu.linkonce.d.*)
        SORT(CONSTRUCTORS)
    }
    
    /* 数据段1 */
    .data1          : { *(.data1) }
    
    /* 数据段结束标记 */
    _edata = .; PROVIDE (edata = .);
    . = .;
    __bss_start = .;
    
    /* BSS段（未初始化数据段） */
    .bss            : {
        *(.dynbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        /* Align here to ensure that the .bss section occupies space up to
        _end.  Align after .bss to ensure correct alignment even if the
        .bss section disappears because there are no input sections.
        FIXME: Why do we need it? When there is no .bss section, we do not
        pad the .data section.  */
        . = ALIGN(. != 0 ? 64 / 8 : 1);
    }
    
    /* 大型BSS段 */
    .lbss   : {
        *(.dynlbss)
        *(.lbss .lbss.* .gnu.linkonce.lb.*)
        *(LARGE_COMMON)
    }
    
    /* 按8字节对齐 */
    . = ALIGN(64 / 8);
    
    /* 设置大型数据段起始位置 */
    . = SEGMENT_START("ldata-segment", .);
    
    /* 大型只读数据段 */
    .lrodata   ALIGN(CONSTANT (MAXPAGESIZE)) + (. & (CONSTANT (MAXPAGESIZE) - 1)) : {
        *(.lrodata .lrodata.* .gnu.linkonce.lr.*)
    }
    
    /* 大型数据段 */
    .ldata   ALIGN(CONSTANT (MAXPAGESIZE)) + (. & (CONSTANT (MAXPAGESIZE) - 1)) : {
        *(.ldata .ldata.* .gnu.linkonce.l.*)
        . = ALIGN(. != 0 ? 64 / 8 : 1);
    }
    
    /* 按8字节对齐 */
    . = ALIGN(64 / 8);
    
    /* 程序结束标记 */
    _end = .; PROVIDE (end = .);
    . = DATA_SEGMENT_END (.);
    
    /* Stabs调试段 */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    .gnu.build.attributes : { *(.gnu.build.attributes .gnu.build.attributes.*) }
    
    /* DWARF调试段 */
    /* DWARF调试段中的符号相对于段的起始位置，所以我们从0开始 */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1扩展 */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1和DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2扩展 */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
    /* DWARF 3 */
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    .debug_ranges   0 : { *(.debug_ranges) }
    /* DWARF 5 */
    .debug_addr     0 : { *(.debug_addr) }
    .debug_line_str 0 : { *(.debug_line_str) }
    .debug_loclists 0 : { *(.debug_loclists) }
    .debug_macro    0 : { *(.debug_macro) }
    .debug_names    0 : { *(.debug_names) }
    .debug_rnglists 0 : { *(.debug_rnglists) }
    .debug_str_offsets 0 : { *(.debug_str_offsets) }
    .debug_sup      0 : { *(.debug_sup) }
    .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
    
    /* 废弃段（不会包含在最终可执行文件中） */
    /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}