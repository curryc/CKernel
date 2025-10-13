# src/platform/x86_64/CMakeLists.txt
# x86_64平台支持

enable_language(ASM_NASM)

# 为x86_64平台创建引导对象
add_library(boot_grub_x86_64 OBJECT boot/boot.S)
set_property(TARGET boot_grub_x86_64 PROPERTY ASM_NASM_FLAGS "-f elf64")

# 添加端口读写库
add_library(port OBJECT port/port.cpp)
target_include_directories(port PUBLIC port/include)

# 创建一个接口库，将所有x86_64平台组件组合在一起
add_library(platform_x86_64 INTERFACE)
target_link_libraries(platform_x86_64 INTERFACE boot_grub_x86_64 port)

# 安装链接脚本
configure_file(linker.ld linker.ld COPYONLY)