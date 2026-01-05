# src/CMakeLists.txt
# 主CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(CKernel)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# 禁用堆栈保护和格式字符串检查
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector -U_FORTIFY_SOURCE -mcmodel=kernel -mno-red-zone -fno-pic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector -U_FORTIFY_SOURCE -mcmodel=kernel -mno-red-zone -fno-pic -fno-exceptions -fno-rtti -O0 -ggdb3 -Wextra")
set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -f elf64")

# 禁用标准库、启动文件、系统库
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -nostartfiles -nodefaultlibs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -nostdlib -nostartfiles -nodefaultlibs")


# 添加头文件目录
include_directories(include)

# 添加子目录
add_subdirectory(platform)
add_subdirectory(kernel)
add_subdirectory(libc)
add_subdirectory(libcxx)
add_subdirectory(libk)
add_subdirectory(selfboot)

# 检查x86_64引导目标是否存在
if(TARGET boot_grub_x86_64)
    # GRUB相关目标
    # 链接成 ELF
    # 先让 CMake 拿到 libgcc 目录
    execute_process(COMMAND ${CMAKE_CXX_COMPILER}
                            -m64 -print-libgcc-file-name
                    OUTPUT_VARIABLE LIBGCC_FULLPATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    get_filename_component(LIBGCC_DIR ${LIBGCC_FULLPATH} DIRECTORY)

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/grub/kernel.elf
        COMMAND ld
            -T ${CMAKE_CURRENT_SOURCE_DIR}/platform/x86_64/linker.ld
            -m elf_x86_64
            -nostdlib
            -L ${LIBGCC_DIR}
            --start-group
            $<TARGET_FILE:boot_grub_x86_64>
            $<TARGET_FILE:kernel>
            --end-group
            -lgcc
            -o ${CMAKE_BINARY_DIR}/grub/kernel.elf
        DEPENDS  kernel libk libc libcxx boot_grub_x86_64 platform
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
    add_custom_target(kernel_elf_target
        DEPENDS ${CMAKE_BINARY_DIR}/grub/kernel.elf
    )

    # 制作 ISO
    set(ISO_DIR ${CMAKE_BINARY_DIR}/grub/isodir)
    set(ISO_FILE ${CMAKE_BINARY_DIR}/grub/kernel.iso)

    add_custom_command(OUTPUT ${ISO_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_DIR}/boot/grub
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub/kernel.elf ${ISO_DIR}/boot/kernel.bin
        COMMAND ${CMAKE_COMMAND} -E echo "set timeout=5" > ${ISO_DIR}/boot/grub/grub.cfg
        COMMAND ${CMAKE_COMMAND} -E echo "set default=0" >> ${ISO_DIR}/boot/grub/grub.cfg
        COMMAND ${CMAKE_COMMAND} -E echo "" >> ${ISO_DIR}/boot/grub/grub.cfg
        COMMAND ${CMAKE_COMMAND} -E echo "menuentry \"CKernel\" {" >> ${ISO_DIR}/boot/grub/grub.cfg
        COMMAND ${CMAKE_COMMAND} -E echo "  multiboot2 /boot/kernel.bin" >> ${ISO_DIR}/boot/grub/grub.cfg
        COMMAND ${CMAKE_COMMAND} -E echo "  boot" >> ${ISO_DIR}/boot/grub/grub.cfg
        COMMAND ${CMAKE_COMMAND} -E echo "}" >> ${ISO_DIR}/boot/grub/grub.cfg
        COMMAND grub-mkrescue -o ${ISO_FILE} ${ISO_DIR} || grub2-mkrescue -o ${ISO_FILE} ${ISO_DIR} || (echo "ERROR: Neither grub-mkrescue nor grub2-mkrescue found!" && echo "Please install grub-pc-bin or grub2-tools package." && exit 1)
        DEPENDS kernel_elf_target
        COMMENT "Creating bootable ISO (GRUB)"
    )

    # GRUB ISO目标
    add_custom_target(grub_iso ALL DEPENDS ${ISO_FILE})

    # GRUB运行目标
    add_custom_target(run-grub
        COMMAND qemu-system-x86_64 -cdrom ${ISO_FILE} 
        -m 128M 
        -machine q35
        -cpu max,,apic=on,-tsc-deadline 
        -global lapic.frequency=1000000   # cpu频率 1 MHz
        # -enable-kvm # 开启硬件模拟，需要平台硬件支持
        -serial stdio 
        -gdb tcp::1234 
        DEPENDS grub_iso
        USES_TERMINAL
    )
endif()

# 添加运行目标
add_custom_target(run DEPENDS run-grub)