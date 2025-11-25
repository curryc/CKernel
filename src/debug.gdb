# GDB调试脚本
target remote :1234
file ./build/grub/kernel.elf
set confirm off
set architecture i386:x86-64
handle SIGTRAP stop print

# 设置断点


# 继续执行
continue

# 当命中断点时，打印相关信息
