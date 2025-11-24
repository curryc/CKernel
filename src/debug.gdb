# GDB调试脚本
target remote :1234
file ./build/grub/kernel.elf
set confirm off
set architecture i386:x86-64
handle SIGTRAP stop print

# 设置断点
break kernel_main.cpp:87
break tui.cpp:55
break intr.cpp:202
break apic.cpp:142

# 继续执行
continue

# 当命中断点时，打印相关信息
commands 1
    echo "Hit kernel_main aaaaaaaaaaaaaaaaa\n"
    backtrace
end

commands 2
    echo "Hit tui.cpp:55\n"
    backtrace
end


commands 3
    echo "Hit interrupts::init\n"
    backtrace
end

commands 4
    echo "Hit apic::init\n"
    backtrace
    print rsdt
end
