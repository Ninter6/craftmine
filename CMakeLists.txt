cmake_minimum_required(VERSION 3.27)
project(craftmine)

set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(FILE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/res/")

add_subdirectory(dep)
add_subdirectory(src)
