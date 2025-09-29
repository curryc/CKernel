# src/platform/CMakeLists.txt
# 平台支持的主CMakeLists.txt

# 添加各平台支持
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/x86_64)
    add_subdirectory(x86_64)
endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/aarch64)
#     add_subdirectory(aarch64)
# endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ia32)
#     add_subdirectory(ia32)
# endif()