# src/selfboot/CMakeLists.txt
# 处理自引导相关代码

# 引导扇区
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/selfboot/bootsect.bin
    COMMAND nasm -f bin ${CMAKE_CURRENT_SOURCE_DIR}/bootsect.asm -o ${CMAKE_BINARY_DIR}/selfboot/bootsect.bin
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bootsect.asm
    COMMENT "Building boot sector (selfboot)"
)

# 第二阶段引导
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/selfboot/stage2.bin
    COMMAND nasm -f bin ${CMAKE_CURRENT_SOURCE_DIR}/bootstage2.asm -o ${CMAKE_BINARY_DIR}/selfboot/stage2.bin
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bootstage2.asm
    COMMENT "Building stage2 (selfboot)"
)

# 简单的自引导内核 (仅用于测试启动)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/selfboot/kernel.bin
    COMMAND ${CMAKE_C_COMPILER} 
            -ffreestanding -O2 -nostdlib -nostartfiles -fno-pie -mno-red-zone -mcmodel=kernel -m64
            -T${CMAKE_CURRENT_SOURCE_DIR}/linkerself.ld
            -o ${CMAKE_BINARY_DIR}/selfboot/kernel.elf
            ${CMAKE_CURRENT_SOURCE_DIR}/mainself.c
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_BINARY_DIR}/selfboot/kernel.elf ${CMAKE_BINARY_DIR}/selfboot/kernel.bin
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mainself.c ${CMAKE_CURRENT_SOURCE_DIR}/linkerself.ld
    COMMENT "Building simple kernel for selfboot"
)

# 1.44 MB 软盘镜像
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/selfboot/floppy.img
    # 空白镜像
    COMMAND dd if=/dev/zero of=${CMAKE_BINARY_DIR}/selfboot/floppy.img bs=512 count=2880 2>/dev/null
    # 写入引导扇区
    COMMAND dd if=${CMAKE_BINARY_DIR}/selfboot/bootsect.bin of=${CMAKE_BINARY_DIR}/selfboot/floppy.img bs=512 count=1 conv=notrunc 2>/dev/null
    # 写入 stage2
    COMMAND dd if=${CMAKE_BINARY_DIR}/selfboot/stage2.bin   of=${CMAKE_BINARY_DIR}/selfboot/floppy.img bs=512 seek=1  count=32 conv=notrunc 2>/dev/null
    # 写入内核
    COMMAND dd if=${CMAKE_BINARY_DIR}/selfboot/kernel.bin   of=${CMAKE_BINARY_DIR}/selfboot/floppy.img bs=512 seek=33 conv=notrunc 2>/dev/null
    DEPENDS ${CMAKE_BINARY_DIR}/selfboot/bootsect.bin ${CMAKE_BINARY_DIR}/selfboot/stage2.bin ${CMAKE_BINARY_DIR}/selfboot/kernel.bin
    COMMENT "Creating floppy.img (selfboot)"
)

# 自引导目标
add_custom_target(selfboot ALL DEPENDS ${CMAKE_BINARY_DIR}/selfboot/floppy.img)

# 自引导运行目标
add_custom_target(run-selfboot
    COMMAND qemu-system-x86_64 -fda ${CMAKE_BINARY_DIR}/selfboot/floppy.img -boot a -m 128M -serial stdio
    DEPENDS selfboot
    USES_TERMINAL
)