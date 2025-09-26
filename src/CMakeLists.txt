cmake_minimum_required(VERSION 3.16)
project(MyKernel ASM C)

# ----------------------------------------------------------
# 1. 工具链
# ----------------------------------------------------------
set(TOOLCHAIN_PREFIX "${TOOLCHAIN_PREFIX}x86_64-linux-gnu-" CACHE STRING "cross-compiler prefix")
set(CMAKE_C_COMPILER   "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_ASM_NASM_COMPILER "nasm")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_PREFIX}objcopy")

# 让 CMake 不再自动加默认 flags
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

# ----------------------------------------------------------
# 2. 统一编译 / 链接标志
# ----------------------------------------------------------
set(KERNEL_CFLAGS
    -std=c11
    -ffreestanding
    -O2
    -Wall -Wextra
    -nostdlib -nostartfiles -fno-pie -mno-red-zone
    -mcmodel=kernel -m64
)

# ----------------------------------------------------------
# 3. 目录与输出
# ----------------------------------------------------------
set(SELFBOOT_DIR ${CMAKE_BINARY_DIR}/selfboot)
set(GRUB_DIR     ${CMAKE_BINARY_DIR}/grub)

file(MAKE_DIRECTORY ${SELFBOOT_DIR} ${GRUB_DIR})

# ----------------------------------------------------------
# 自引导链（selfboot）- 仅用于启动测试
# ----------------------------------------------------------
# 引导扇区
add_custom_command(OUTPUT ${SELFBOOT_DIR}/bootsect.bin
    COMMAND nasm -f bin ${CMAKE_SOURCE_DIR}/selfboot/bootsect.asm -o ${SELFBOOT_DIR}/bootsect.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/selfboot/bootsect.asm
    COMMENT "Building boot sector (selfboot)"
)
# 第二阶段引导
add_custom_command(OUTPUT ${SELFBOOT_DIR}/stage2.bin
    COMMAND nasm -f bin ${CMAKE_SOURCE_DIR}/selfboot/bootstage2.asm -o ${SELFBOOT_DIR}/stage2.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/selfboot/bootstage2.asm
    COMMENT "Building stage2 (selfboot)"
)
# 简单的自引导内核 (仅用于测试启动)
add_custom_command(OUTPUT ${SELFBOOT_DIR}/kernel.bin
    COMMAND ${CMAKE_C_COMPILER} 
            -ffreestanding -O2 -nostdlib -nostartfiles -fno-pie -mno-red-zone -mcmodel=kernel -m64
            -T${CMAKE_SOURCE_DIR}/selfboot/linkerself.ld
            -o ${SELFBOOT_DIR}/kernel.elf
            ${CMAKE_SOURCE_DIR}/selfboot/mainself.c
    COMMAND ${CMAKE_OBJCOPY} -O binary ${SELFBOOT_DIR}/kernel.elf ${SELFBOOT_DIR}/kernel.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/selfboot/mainself.c ${CMAKE_SOURCE_DIR}/selfboot/linkerself.ld
    COMMENT "Building simple kernel for selfboot"
)
# 1.44 MB 软盘镜像
add_custom_command(OUTPUT ${SELFBOOT_DIR}/floppy.img
    # 空白镜像
    COMMAND dd if=/dev/zero of=${SELFBOOT_DIR}/floppy.img bs=512 count=2880 2>/dev/null
    # 写入引导扇区
    COMMAND dd if=${SELFBOOT_DIR}/bootsect.bin of=${SELFBOOT_DIR}/floppy.img bs=512 count=1 conv=notrunc 2>/dev/null
    # 写入 stage2
    COMMAND dd if=${SELFBOOT_DIR}/stage2.bin   of=${SELFBOOT_DIR}/floppy.img bs=512 seek=1  count=32 conv=notrunc 2>/dev/null
    # 写入内核
    COMMAND dd if=${SELFBOOT_DIR}/kernel.bin   of=${SELFBOOT_DIR}/floppy.img bs=512 seek=33 conv=notrunc 2>/dev/null
    DEPENDS ${SELFBOOT_DIR}/bootsect.bin ${SELFBOOT_DIR}/stage2.bin ${SELFBOOT_DIR}/kernel.bin
    COMMENT "Creating floppy.img (selfboot)"
)
add_custom_target(selfboot ALL DEPENDS ${SELFBOOT_DIR}/floppy.img)

# ----------------------------------------------------------
# GRUB 链 - 主要维护
# ----------------------------------------------------------
# 64 位 ELF 引导对象
enable_language(ASM_NASM)
add_library(boot_grub OBJECT boot/boot.asm)
set_property(TARGET boot_grub PROPERTY ASM_NASM_FLAGS "-f elf64")

# 内核静态库（GRUB 版）
# 添加头文件目录
include_directories(include)
add_library(kernel_grub STATIC
    kernel/main.c
    kernel/pmm.c
)
target_compile_options(kernel_grub PRIVATE
    ${KERNEL_CFLAGS}
)
set_target_properties(kernel_grub PROPERTIES
    ARCHIVE_OUTPUT_NAME kernel_grub
    ARCHIVE_OUTPUT_DIRECTORY ${GRUB_DIR}
)
# 链接成 ELF
add_custom_command(OUTPUT ${GRUB_DIR}/kernel.elf
    COMMAND ${CMAKE_C_COMPILER} -T${CMAKE_SOURCE_DIR}/boot/linker.ld -no-pie -nostdlib -mcmodel=large -m64 $<TARGET_OBJECTS:boot_grub> ${GRUB_DIR}/libkernel_grub.a -o ${GRUB_DIR}/kernel.elf
    DEPENDS kernel_grub boot_grub
    COMMENT "Linking kernel.elf (GRUB)"
)

# 制作 ISO
set(ISO_DIR ${GRUB_DIR}/isodir)
set(ISO_FILE ${GRUB_DIR}/kernel.iso)

file(MAKE_DIRECTORY ${ISO_DIR}/grub)

add_custom_command(OUTPUT ${ISO_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_DIR}/boot/grub
    COMMAND ${CMAKE_COMMAND} -E copy ${GRUB_DIR}/kernel.elf ${ISO_DIR}/boot/kernel.bin
    COMMAND ${CMAKE_COMMAND} -E echo "set timeout=5" > ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "set default=0" >> ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "" >> ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "menuentry \"My Kernel\" {" >> ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "  multiboot2 /boot/kernel.bin" >> ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "  boot" >> ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "}" >> ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND grub-mkrescue -o ${ISO_FILE} ${ISO_DIR} || grub2-mkrescue -o ${ISO_FILE} ${ISO_DIR} || (echo "ERROR: Neither grub-mkrescue nor grub2-mkrescue found!" && echo "Please install grub-pc-bin or grub2-tools package." && exit 1)
    DEPENDS ${GRUB_DIR}/kernel.elf
    COMMENT "Creating bootable ISO (GRUB)"
)

add_custom_target(grub ALL DEPENDS ${ISO_FILE})

# ----------------------------------------------------------
# 运行 / 调试
# ----------------------------------------------------------
# 运行
add_custom_target(run-selfboot
    COMMAND qemu-system-x86_64 -fda ${SELFBOOT_DIR}/floppy.img -boot a -m 128M -serial stdio
    DEPENDS selfboot
    USES_TERMINAL
)

add_custom_target(run-grub
    COMMAND qemu-system-x86_64 -cdrom ${ISO_FILE} -m 128M -serial stdio
    DEPENDS grub
    USES_TERMINAL
)

# 默认 run 指向 grub 版
add_custom_target(run DEPENDS run-grub)

# 调试
add_custom_target(debug-selfboot
    COMMAND qemu-system-x86_64 -fda ${SELFBOOT_DIR}/floppy.img -m 128M -serial stdio -s -S &
    COMMAND gdb -ex "target remote localhost:1234" -ex "symbol-file ${SELFBOOT_DIR}/kernel.elf"
    DEPENDS selfboot
    USES_TERMINAL
)

add_custom_target(debug-grub
    COMMAND qemu-system-x86_64 -cdrom ${ISO_FILE} -m 128M -serial stdio -s -S &
    COMMAND gdb -ex "target remote localhost:1234" -ex "symbol-file ${GRUB_DIR}/kernel.elf"
    DEPENDS grub
    USES_TERMINAL
)

# ----------------------------------------------------------
# 清理
# ----------------------------------------------------------
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning entire build directory"
)