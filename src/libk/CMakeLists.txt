project(libk)
# 查找C++源文件
aux_source_directory(./src/. libk_src)

set(libk_src ${libk_src})
# 设置包含目录
set(LIBK_INCLUDE_DIRS 
    ${libk_SOURCE_DIR}/include
)
# Add library
add_library(${PROJECT_NAME} OBJECT ${libk_src})

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${LIBK_INCLUDE_DIRS}
)

target_link_libraries(libk
    libc
    libcxx
    platform
)
