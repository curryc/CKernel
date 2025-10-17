# src/platform/x86_64/CMakeLists.txt
# x86_64平台支持

enable_language(ASM)

# 设置汇编编译器标志
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m64")

# 为x86_64平台创建引导对象，明确列出两个汇编文件
add_library(boot_grub_x86_64 STATIC 
    idt/idt.S boot/boot.S )

# 设置目标属性
set_target_properties(boot_grub_x86_64 PROPERTIES
    POSITION_INDEPENDENT_CODE OFF
)

# 添加端口读写库
set(CMAKE_CXX_STANDARD 11)
set_source_files_properties(port/port.cpp PROPERTIES LANGUAGE CXX)
add_library(port STATIC port/port.cpp)
target_include_directories(port PUBLIC port/include)

# 创建一个接口库，将所有x86_64平台组件组合在一起
add_library(platform_x86_64 INTERFACE)
target_link_libraries(platform_x86_64 INTERFACE boot_grub_x86_64 port)

# 安装链接脚本
configure_file(linker.ld linker.ld COPYONLY)