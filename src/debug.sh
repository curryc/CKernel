#!/bin/bash
QEMU="qemu-system-x86_64 "
OPTS="-cdrom ./build/grub/kernel.iso
        -m 128M 
        -serial stdio 
        -no-reboot
        -no-shutdown
        -d int
        -gdb tcp::1234"
echo "Starting QEMU with serial + gdbstub..."
$QEMU $OPTS &
QEMU_PID=$!

sleep 1

gdb -ex "target remote :1234" \
    -ex "set confirm off" \
    -ex "set architecture i386:x86-64" \
    -ex "handle SIGTRAP stop print" \
    -ex "file ./build/grub/kernel.elf" \ 
    -ex "b _start_64" \
    -ex "c" \
    -ex "bt"
kill $QEMU_PID 2>/dev/null