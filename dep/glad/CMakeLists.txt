add_library(glad STATIC glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(glad PRIVATE -O3 -march=native)
