# src/platform/x86_64/CMakeLists.txt
# x86_64平台支持

enable_language(ASM_NASM)

# 为x86_64平台创建引导对象
add_library(boot_grub_x86_64 OBJECT boot/boot.asm)
set_property(TARGET boot_grub_x86_64 PROPERTY ASM_NASM_FLAGS "-f elf64")

# 添加端口读写库
add_library(port OBJECT port/port.cpp)
target_include_directories(port PRIVATE port/include)

# 安装链接脚本
configure_file(linker.ld linker.ld COPYONLY)