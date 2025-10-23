project(common)

aux_source_directory(./src cxx_src)

add_library(common STATIC ${cxx_src})

target_include_directories(${PROJECT_NAME} PUBLIC include)
# 链接
target_link_libraries(${PROJECT_NAME} PUBLIC 
    libc 
    libcxx
    platform
)

# 拒绝外部C++库
target_compile_options(common PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -std=c++11
        -ffreestanding
        -O2
        -Wall -Wextra
        -Wno-int-to-pointer-cast
        -nostdlib -nostartfiles -fno-pie -mno-red-zone
        -fno-exceptions -fno-rtti
    >
)
# 禁用位置无关代码
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE OFF)