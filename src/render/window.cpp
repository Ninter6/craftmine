//
// Created by Ninter6 on 2024/7/15.
//

#include "window.hpp"

#include "world/world.hpp"

#include <iostream>

auto GLFW = glfw::init();

Window::Window(const WindowInfo& info) : size(info.extent) {
    init_window(info);
    init_event();
}

void Window::init_window(const WindowInfo& info) {
    glfw::WindowHints{
        .contextVersionMajor = 4,
        .contextVersionMinor = 1,
#ifdef __APPLE__
        .openglForwardCompat = true,
#endif
        .openglProfile = glfw::OpenGlProfile::Core }.apply();
#ifdef __APPLE__
    window = {size.x/2, size.y/2, info.title};
#else
    window = {size.x, size.y, info.title};
#endif

    glfw::makeContextCurrent(window);

    window.setInputModeCursor(glfw::CursorMode::Disabled);

    static auto GLAD = ([]{
        if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return;
        std::cerr << "Failed to initialize GLAD" << std::endl;
        std::terminate();
    }(), 0);
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    glViewport(0, 0, size.x, size.y);
}

void Window::init_event() {
    eventor = std::make_unique<Eventor>(
        std::make_shared<Listener>(
            [&](int k) {return window.getKey((decltype(glfw::KeyCode::Unknown))k);},
            [&](int m) {return window.getMouseButton((decltype(glfw::MouseButton::Left))m);},
            [&](double& x, double& y) {std::tie(x, y) = window.getCursorPos();}
        )
    );

    eventor->add_event({0, 0, 256, 0}, [&, cur = false](auto&& lsn) mutable {
        if (!lsn.IsKeyPressed(256)) return;
        cur = !cur;
        window.setInputModeCursor(cur ? glfw::CursorMode::Normal : glfw::CursorMode::Disabled);
    });
}

bool Window::should_close() {
    return window.shouldClose();
}

void Window::next_frame() {
    window.swapBuffers();
    glfw::pollEvents();
}
