# src/platform/CMakeLists.txt
project(platform)

add_library(platform INTERFACE)

# 添加各平台支持
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/x86_64)
    add_subdirectory(x86_64)
    target_link_libraries(platform INTERFACE platform_x86_64)
endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/aarch64)
#     add_subdirectory(aarch64)
# endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ia32)
#     add_subdirectory(ia32)
# endif()