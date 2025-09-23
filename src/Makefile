# ---------- 工具链 ----------
CROSS_COMPILE := x86_64-linux-gnu-
CC            := $(CROSS_COMPILE)gcc
AS            := nasm
OBJCOPY       := $(CROSS_COMPILE)objcopy

# ---------- 编译 flags ----------
CFLAGS   := -std=c11 -ffreestanding -O2 -Wall -Wextra \
            -nostdlib -nostartfiles -fno-pie -mno-red-zone \
            -mcmodel=kernel -m64
LDFLAGS  := -T boot/linker.ld -no-pie -nostdlib -mcmodel=large -m64

# ---------- 文件 ----------
KERNEL_ELF   := build/kernel.elf
ISO          := build/kernel.iso

# ---------- 默认目标 ----------
all: $(ISO)

# ---------- 内核 ----------
$(KERNEL_ELF): boot/boot.asm kernel/main.c boot/linker.ld
	@mkdir -p build
	$(AS) -f elf64 boot/boot.asm -o build/boot.o
	$(CC) $(CFLAGS) -c kernel/main.c -o build/main.o
	$(CC) $(LDFLAGS) -o $@ build/boot.o build/main.o

# ---------- 可启动 ISO (使用 GRUB) ----------
$(ISO): $(KERNEL_ELF)
	@mkdir -p build/isodir/boot/grub
	cp $(KERNEL_ELF) build/isodir/boot/kernel.bin
	@echo 'set timeout=5' > build/isodir/boot/grub/grub.cfg
	@echo 'set default=0' >> build/isodir/boot/grub/grub.cfg
	@echo '' >> build/isodir/boot/grub/grub.cfg
	@echo 'menuentry "My Kernel" {' >> build/isodir/boot/grub/grub.cfg
	@echo '  multiboot2 /boot/kernel.bin' >> build/isodir/boot/grub/grub.cfg
	@echo '  boot' >> build/isodir/boot/grub/grub.cfg
	@echo '}' >> build/isodir/boot/grub/grub.cfg
	
	# 创建可启动ISO
	@if command -v grub-mkrescue >/dev/null 2>&1; then \
		grub-mkrescue -o $@ build/isodir; \
	elif command -v grub2-mkrescue >/dev/null 2>&1; then \
		grub2-mkrescue -o $@ build/isodir; \
	else \
		echo "ERROR: Neither grub-mkrescue nor grub2-mkrescue found!"; \
		echo "Please install grub-pc-bin or grub2-tools package."; \
		exit 1; \
	fi

# ---------- 运行 ----------
run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -serial stdio -d int,cpu_reset,guest_errors -D qemu.log

# ---------- debug ----------
debug: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -serial stdio -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file build/kernel.elf"

# ---------- 清理 ----------
clean:
	rm -rf build/*

.PHONY: all clean run