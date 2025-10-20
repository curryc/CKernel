#!/bin/bash
QEMU="qemu-system-x86_64"
OPTS="-cdrom ./build/grub/kernel.iso
        -m 128M 
        -serial mon:stdio 
        -no-reboot
        -no-shutdown
        -d cpu_reset,guest_errors
        -gdb tcp::1234"
echo "Starting QEMU with serial + gdbstub..."
$QEMU $OPTS 2>&1 &
QEMU_PID=$!

# 等待内核输出
sleep 5

# 如果内核还在运行，发送信号终止
if kill -0 $QEMU_PID 2>/dev/null; then
    echo "Terminating QEMU..."
    kill $QEMU_PID 2>/dev/null
fi
