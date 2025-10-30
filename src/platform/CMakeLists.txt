# src/platform/CMakeLists.txt
project(platform)

enable_language(ASM)
# 设置汇编编译器标志
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m64")
# 检查x86_64平台文件是否存在
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/x86_64/boot/boot.S)
    # 为x86_64平台创建引导对象，明确列出两个汇编文件
    add_library(boot_grub_x86_64 STATIC 
        x86_64/idt/idt.S 
        x86_64/boot/boot.S
    )

    # 设置目标属性
    set_target_properties(boot_grub_x86_64 PROPERTIES
        POSITION_INDEPENDENT_CODE OFF
    )

    # 添加其他库
    add_library(platform OBJECT 
        x86_64/src/c_time.cpp
        x86_64/src/c_port.cpp
        x86_64/src/port.cpp)
    # 设置目标属性
    set_target_properties(platform PROPERTIES
        OSITION_INDEPENDENT_CODE OFF
        CXX_STANDARD 11  # 设置 C++ 标准为 C++11
        CXX_STANDARD_REQUIRED ON  # 强制要求指定的 C++ 标准
    )
    target_include_directories(platform PUBLIC x86_64/include)

    # 安装链接脚本
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/x86_64/linker.ld)
        configure_file(x86_64/linker.ld linker.ld COPYONLY)
    endif()
    
else()
    message(WARNING "x86_64 platform files not found. Platform library will not be built.")
endif()
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/aarch64)
#     add_subdirectory(aarch64)
# endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ia32)
#     add_subdirectory(ia32)
# endif()