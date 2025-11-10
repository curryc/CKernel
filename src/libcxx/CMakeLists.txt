PROJECT(libcxx CXX)

# 查找C++源文件
aux_source_directory(${libcxx_SOURCE_DIR}/src cxx_src)

set(libcxx_src ${cxx_src})
# 设置包含目录
set(LIBCXX_INCLUDE_DIRS 
    ${libcxx_SOURCE_DIR}/include
)

# 创建库
if(libcxx_src)
    # 如果有源文件则创建STATIC库
    add_library(${PROJECT_NAME} OBJECT ${libcxx_src})
    
    # 设置包含目录
    target_include_directories(${PROJECT_NAME} PUBLIC 
        ${LIBCXX_INCLUDE_DIRS}
    )
    
    # 链接到libc
    target_link_libraries(${PROJECT_NAME} PUBLIC libc)
else()
    # 如果没有源文件，则创建一个空的INTERFACE库
    add_library(${PROJECT_NAME} INTERFACE)
    
    # 为INTERFACE库设置包含目录
    target_include_directories(${PROJECT_NAME} INTERFACE 
        ${LIBCXX_INCLUDE_DIRS}
    )
    
    # INTERFACE库依赖libc
    target_link_libraries(${PROJECT_NAME} INTERFACE libc)
endif()
