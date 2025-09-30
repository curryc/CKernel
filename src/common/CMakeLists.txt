project(common)

add_library(common SHARED
  tui.cpp
)

target_include_directories(common PUBLIC include)