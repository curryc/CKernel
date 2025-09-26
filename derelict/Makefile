Makefile
# ---------- 工具链 ----------
CROSS_COMPILE := x86_64-linux-gnu-
CC            := $(CROSS_COMPILE)gcc
AS            := nasm
OBJCOPY       := $(CROSS_COMPILE)objcopy

# ---------- 编译 flags ----------
CFLAGS   := -std=c11 -ffreestanding -O2 -Wall -Wextra \
            -nostdlib -nostartfiles -fno-pie -mno-red-zone \
            -mcmodel=kernel -m64
LDFLAGS_SELFBOOT  := -T boot/linkerself.ld -no-pie -nostdlib -mcmodel=large -m64
LDFLAGS_GRUB      := -T boot/linker.ld -no-pie -nostdlib -mcmodel=large -m64

# ---------- 文件 ----------
# 自引导方式文件
BOOTSECT_BIN      := build/selfboot/bootsect.bin
STAGE2_BIN        := build/selfboot/stage2.bin
KERNEL_SELF_ELF   := build/selfboot/kernel.elf
KERNEL_SELF_BIN   := build/selfboot/kernel.bin
FLOPPY_IMG        := build/selfboot/floppy.img

# GRUB引导方式文件
KERNEL_GRUB_ELF   := build/grub/kernel.elf
ISO               := build/grub/kernel.iso

# ---------- 默认目标 ----------
all: selfboot

# ---------- 自引导方式目标 ----------
selfboot: $(FLOPPY_IMG)

# ---------- GRUB引导方式目标 ----------
grub: $(ISO)

# ---------- 引导扇区 ----------
$(BOOTSECT_BIN): boot/bootsect.asm
	@mkdir -p build/selfboot
	$(AS) -f bin boot/bootsect.asm -o $@

# ---------- 第二阶段引导 ----------
$(STAGE2_BIN): boot/bootstage2.asm
	@mkdir -p build/selfboot
	$(AS) -f bin boot/bootstage2.asm -o $@

# ---------- 自引导方式内核 ----------
$(KERNEL_SELF_ELF): kernel/main.c kernel/pmm.c kernel/pmm.h boot/linkerself.ld
	@mkdir -p build/selfboot
	$(CC) $(CFLAGS) -c kernel/main.c -o build/selfboot/main.o
	$(CC) $(CFLAGS) -c kernel/pmm.c -o build/selfboot/pmm.o
	$(CC) $(LDFLAGS_SELFBOOT) -o $@ build/selfboot/main.o build/selfboot/pmm.o

$(KERNEL_SELF_BIN): $(KERNEL_SELF_ELF)
	$(OBJCOPY) -O binary $< $@

# ---------- 可启动镜像 ----------
$(FLOPPY_IMG): $(BOOTSECT_BIN) $(STAGE2_BIN) $(KERNEL_SELF_BIN)
	@mkdir -p build/selfboot
	# 创建1.44MB软盘镜像
	dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	# 写入引导扇区
	dd if=$(BOOTSECT_BIN) of=$@ bs=512 count=1 conv=notrunc 2>/dev/null
	# 写入第二阶段引导程序
	dd if=$(STAGE2_BIN) of=$@ bs=512 seek=1 count=32 conv=notrunc 2>/dev/null
	# 写入内核
	dd if=$(KERNEL_SELF_BIN) of=$@ bs=512 seek=33 conv=notrunc 2>/dev/null

# ---------- GRUB引导方式内核 ----------
$(KERNEL_GRUB_ELF): boot/boot.asm kernel/main.c kernel/pmm.c kernel/pmm.h boot/linker.ld
	@mkdir -p build/grub
	$(AS) -f elf64 boot/boot.asm -o build/grub/boot.o
	$(CC) $(CFLAGS) -c kernel/main.c -o build/grub/main.o
	$(CC) $(CFLAGS) -c kernel/pmm.c -o build/grub/pmm.o
	$(CC) $(LDFLAGS_GRUB) -o $@ build/grub/boot.o build/grub/main.o build/grub/pmm.o

# ---------- GRUB引导方式ISO镜像 ----------
$(ISO): $(KERNEL_GRUB_ELF)
	@mkdir -p build/grub/isodir/boot/grub
	cp $(KERNEL_GRUB_ELF) build/grub/isodir/boot/kernel.bin
	@echo 'set timeout=5' > build/grub/isodir/boot/grub/grub.cfg
	@echo 'set default=0' >> build/grub/isodir/boot/grub/grub.cfg
	@echo '' >> build/grub/isodir/boot/grub/grub.cfg
	@echo 'menuentry "My Kernel" {' >> build/grub/isodir/boot/grub/grub.cfg
	@echo '  multiboot2 /boot/kernel.bin' >> build/grub/isodir/boot/grub/grub.cfg
	@echo '  boot' >> build/grub/isodir/boot/grub/grub.cfg
	@echo '}' >> build/grub/isodir/boot/grub/grub.cfg
	
	# 创建可启动ISO
	@if command -v grub-mkrescue >/dev/null 2>&1; then \
		grub-mkrescue -o $@ build/grub/isodir; \
	elif command -v grub2-mkrescue >/dev/null 2>&1; then \
		grub2-mkrescue -o $@ build/grub/isodir; \
	else \
		echo "ERROR: Neither grub-mkrescue nor grub2-mkrescue found!"; \
		echo "Please install grub-pc-bin or grub2-tools package."; \
		exit 1; \
	fi

# ---------- 运行自引导方式 ----------
run-selfboot: $(FLOPPY_IMG)
	qemu-system-x86_64 -fda build/selfboot/floppy.img -boot a -m 128M -serial stdio

# ---------- 运行GRUB引导方式 ----------
run-grub: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -serial stdio

# ---------- 默认运行方式 ----------
run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -serial stdio

# ---------- debug自引导方式 ----------
debug-selfboot: $(FLOPPY_IMG)
	qemu-system-x86_64 -fda $< -m 128M -serial stdio -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file build/selfboot/kernel.elf"

# ---------- debug GRUB引导方式 ----------
debug-grub: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -serial stdio -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file build/grub/kernel.elf"

# ---------- 清理 ----------
clean:
	rm -rf build/*

.PHONY: all clean run run-selfboot run-grub selfboot grub debug-selfboot debug-grub