# GDB调试脚本
target remote :1234
set confirm off
set pagination off

# 设置断点
break kernel_main
break test
break INTERRUPTS::init
break port_outb

# 继续执行
continue

# 当命中断点时，打印相关信息
commands 1
    echo "Hit kernel_main\n"
    backtrace
    continue
end

commands 2
    echo "Hit test\n"
    backtrace
    continue
end

commands 3
    echo "Hit INTERRUPTS::init\n"
    backtrace
    print idtr
    print idt[0]
    continue
end

commands 4
    echo "Hit port_outb\n"
    backtrace
    print _port
    print _data
    continue
end
