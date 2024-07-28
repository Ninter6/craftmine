file(GLOB SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp")

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} glad stb GLFWPP)