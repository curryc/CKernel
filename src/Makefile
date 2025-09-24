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
BOOTSECT_BIN := build/bootsect.bin
STAGE2_BIN   := build/stage2.bin
KERNEL_ELF   := build/kernel.elf
KERNEL_BIN   := build/kernel.bin
ISO          := build/kernel.iso


# ---------- 引导扇区 ----------
$(BOOTSECT_BIN): boot/bootsect.asm
	@mkdir -p build
	$(AS) -f bin boot/bootsect.asm -o $@

# ---------- 第二阶段引导 ----------
$(STAGE2_BIN): boot/bootstage2.asm
	@mkdir -p build
	$(AS) -f bin boot/bootstage2.asm -o $@

# ---------- 内核 ----------
$(KERNEL_ELF): boot/boot.asm kernel/main.c boot/linker.ld
	@mkdir -p build
	$(AS) -f elf64 boot/boot.asm -o build/boot.o
	$(CC) $(CFLAGS) -c kernel/main.c -o build/main.o
	$(CC) $(LDFLAGS) -o $@ build/boot.o build/main.o

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# ---------- 可启动镜像 ----------
build/floppy.img: $(BOOTSECT_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	@mkdir -p build
	# 创建1.44MB软盘镜像
	dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	# 写入引导扇区
	dd if=$(BOOTSECT_BIN) of=$@ bs=512 count=1 conv=notrunc 2>/dev/null
	# 写入第二阶段引导程序
	dd if=$(STAGE2_BIN) of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	# 写入内核
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=10 conv=notrunc 2>/dev/null

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

# ---------- 默认目标 ----------
all: $(ISO)

# ---------- 运行 (软盘方式) ----------
run-floppy: build/floppy.img
	qemu-system-x86_64 -fda $< -m 128M -serial stdio -d int,cpu_reset,guest_errors -D qemu.log

# ---------- 运行 (ISO方式) ----------
run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -serial stdio -d int,cpu_reset,guest_errors -D qemu.log

# ---------- debug ----------
debug: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -serial stdio -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file build/kernel.elf"

# ---------- 清理 ----------
clean:
	rm -rf build/*

.PHONY: all clean run run-floppy