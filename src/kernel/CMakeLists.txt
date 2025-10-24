# src/kernel/CMakeLists.txt
# 处理内核核心代码

# 添加头文件目录
PROJECT(kernel CXX)

aux_source_directory(${kernel_SOURCE_DIR}/src/. kernel_src)

add_library(kernel STATIC ${kernel_src})

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${kernel_SOURCE_DIR}/include
)

# 链接
target_link_libraries(kernel PRIVATE 
    platform
    libc 
    libcxx
    common
)

set_target_properties(kernel PROPERTIES
    ARCHIVE_OUTPUT_NAME kernel
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/grub
)