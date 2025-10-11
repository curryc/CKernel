project(common)

aux_source_directory(${common_SOURCE_DIR}/src cxx_src)

add_library(common STATIC ${cxx_src})

target_include_directories(${PROJECT_NAME} PUBLIC include)
# 链接
target_link_libraries(${PROJECT_NAME} PUBLIC 
    libc 
    libcxx
    platform
    port
)

# 禁用位置无关代码
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE OFF)