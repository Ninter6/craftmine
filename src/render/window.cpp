//
// Created by Ninter6 on 2024/7/15.
//

#include "window.hpp"

#include "world/world.hpp"

#include <iostream>

auto GLFW = glfw::init();

Window::Window(const WindowInfo& info) : size(info.extent) {
    init_window(info);
    init_render(info.extent.x, info.extent.y);
    init_event();
}

void Window::init_window(const WindowInfo& info) {
    glfw::WindowHints{
        .samples = 4,
        .contextVersionMajor = 3,
        .contextVersionMinor = 3,
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
    glViewport(0, 0, size.x, size.y);
}

void Window::init_render(int w, int h) {
    renderer = std::make_unique<Renderer>(w, h);
}

void Window::init_event() {
    eventor = std::make_unique<Eventor>(
        std::make_shared<Listener>(
            [&](int k) {return window.getKey((decltype(glfw::KeyCode::Unknown))k);},
            [&](int m) {return window.getMouseButton((decltype(glfw::MouseButton::Left))m);},
            [&](double& x, double& y) {std::tie(x, y) = window.getCursorPos();}
        )
    );

    eventor->add_event({{}, {}, 256}, [&, cur = false](auto&& lsn) mutable {
        if (!lsn.IsKeyPressed(256)) return;
        cur = !cur;
        window.setInputModeCursor(cur ? glfw::CursorMode::Normal : glfw::CursorMode::Disabled);
    });

    renderer->init_event(*this, *eventor);
}

void Window::loop() {
    while (!window.shouldClose()) {
        eventor->Update();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render();

        window.swapBuffers();
        glfw::pollEvents();
    }
}

void Window::render() {
    static World world{{
        .seed = 114514,
        .camera = renderer->getCamera()
    }};
    static auto _ = ([&]{
        eventor->add_event({{}, {}, {}, 0, {}}, [&](auto&& lsn) {
            if (lsn.IsMouseButtonReleased(1))
                world.set_camera_target_block(BlockType::air, false);
            else if (lsn.IsMouseButtonReleased(2))
                world.set_camera_target_block(BlockType::stone, true);
        });
    }(), 0);

    world.update();

    renderer->render(world.get_draw_data());
}
