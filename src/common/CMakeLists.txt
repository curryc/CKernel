project(common)

aux_source_directory(./src cxx_src)

add_library(common OBJECT ${cxx_src})

target_include_directories(${PROJECT_NAME} PUBLIC include)
# 链接
target_link_libraries(${PROJECT_NAME} PUBLIC 
    libc 
    libcxx
    platform
)
