# src/kernel/CMakeLists.txt
# 处理内核核心代码

# 添加头文件目录
include_directories(./include)

aux_source_directory(./src cxx_src)

add_library(kernel_grub STATIC ${cxx_src})

# 链接
target_link_libraries(kernel_grub PRIVATE 
    platform
    libc 
    libcxx
    common
)

# 设置C++编译选项
target_compile_options(kernel_grub PRIVATE
    $<$<COMPILE_LANGUAGE:C>:
        -std=c11
        -ffreestanding
        -O2
        -Wall -Wextra
        -Wno-int-to-pointer-cast
        -nostdlib -nostartfiles -fno-pie -mno-red-zone
        -mcmodel=kernel -m64
    >
    $<$<COMPILE_LANGUAGE:CXX>:
        -std=c++11
        -ffreestanding
        -O2
        -Wall -Wextra
        -Wno-int-to-pointer-cast
        -nostdlib -nostartfiles -fno-pie -mno-red-zone
        -mcmodel=kernel -m64
        -fno-exceptions -fno-rtti
        -fvisibility=default
    >
)

# 禁用位置无关代码
set_property(TARGET kernel_grub PROPERTY POSITION_INDEPENDENT_CODE OFF)

set_target_properties(kernel_grub PROPERTIES
    ARCHIVE_OUTPUT_NAME kernel_grub
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/grub
)