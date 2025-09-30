PROJECT(libc C)

# 查找各个子目录下的源文件
aux_source_directory(${libc_SOURCE_DIR}/src/stdio stdio_src)
aux_source_directory(${libc_SOURCE_DIR}/src/string string_src)

# 合并所有源文件
set(libc_src ${stdio_src} ${string_src})

# 创建库
if(libc_src)
    # 如果有源文件则创建OBJECT库
    add_library(${PROJECT_NAME} STATIC ${libc_src})
    
    # 设置包含目录
    target_include_directories(${PROJECT_NAME} PUBLIC 
        ${libc_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
    )
    
    # 设置位置无关代码属性
    set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
else()
    # 如果没有源文件，则创建一个空的INTERFACE库
    add_library(${PROJECT_NAME} INTERFACE)
    
    # 为INTERFACE库设置包含目录
    target_include_directories(${PROJECT_NAME} PUBLIC 
        ${libc_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
    )
endif()

# 提供一个变量给其他模块使用
set(LIBC_INCLUDE_DIRS ${LIBC_INCLUDE_DIRS} PARENT_SCOPE)