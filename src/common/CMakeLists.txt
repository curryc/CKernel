project(common)

aux_source_directory(${common_SOURCE_DIR} cxx_src)

add_library(common STATIC ${cxx_src})

target_include_directories(${PROJECT_NAME} PUBLIC include)
# 链接
target_link_libraries(${PROJECT_NAME} PUBLIC 
    libc 
    libcxx
    platform
    port
)

