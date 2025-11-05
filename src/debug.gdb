# GDB调试脚本
target remote :1234
file ./build/grub/kernel.elf
set confirm off
set architecture i386:x86-64
handle SIGTRAP stop print

# 设置断点
break kernel_main
break intr.cpp:202

# 继续执行
continue

# 当命中断点时，打印相关信息
commands 1
    echo "Hit kernel_main\n"
    backtrace
    continue
end

commands 2
    echo "Hit interrupts::init\n"
    backtrace
    next
end
