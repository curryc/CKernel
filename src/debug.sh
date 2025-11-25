#!/bin/bash

# 定义 QEMU 及其启动参数
QEMU="qemu-system-x86_64"
OPTS="-cdrom ./build/grub/kernel.iso \
      -m 128M \
      -serial mon:stdio \
      -no-reboot \
      -no-shutdown \
      -d int,cpu_reset,guest_errors \
      -gdb tcp::1234 \
      -no-reboot \
      -no-shutdown"

# 启动 QEMU
echo "Starting QEMU with serial + gdbstub..."
$QEMU $OPTS 2>&1 &
QEMU_PID=$!

# 等待一段时间，确保 QEMU 启动完成并监听 gdbstub
sleep 1

# 检查 QEMU 是否仍在运行
if kill -0 $QEMU_PID 2>/dev/null; then
    echo "QEMU is running. Starting GDB with debug script..."

    # 使用 GDB 加载 debug.gdb 脚本进行调试
    gdb -x ./debug.gdb

    # 调试结束后终止 QEMU
    echo "Terminating QEMU..."
    kill $QEMU_PID 2>/dev/null
else
    echo "QEMU failed to start."
fi
