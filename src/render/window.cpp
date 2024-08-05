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
    init_world();
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

void Window::init_render(int w, int h) {
    renderer = std::make_unique<Renderer>(w, h);
}

void Window::init_world() {
    world = std::make_unique<World>(WorldInitInfo{
        .seed = 114514,
        .camera = renderer->getCamera()
    });
}

BlockType B[] {
    BlockType::stone,
    BlockType::wooden_plank,
    BlockType::log,
    BlockType::glass_nt,
    BlockType::glass_red,
    BlockType::glass_green,
    BlockType::glass_blue,
    BlockType::water,
    BlockType::sand,
};
int b = 0;

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

    eventor->add_event({{}, {}, {}, 0, {}}, [&](auto&& lsn) {
        if (lsn.IsMouseButtonReleased(1))
            world->set_camera_target_block(BlockType::air, false);
        else if (lsn.IsMouseButtonReleased(2))
            world->set_camera_target_block(B[b], true);
    });
}

void Window::loop() {
    using cl = std::chrono::high_resolution_clock;
    auto t = cl::now();
    int count = 0;
    while (!window.shouldClose()) {
        update();
        render();

        window.swapBuffers();
        glfw::pollEvents();

        count++;
        if (cl::now() - t > std::chrono::seconds{1}) {
            std::printf("\rFPS: %d", count);
            t = cl::now();
            count = 0;
        }
        if (window.getKey(glfw::KeyCode::One))
            b = 0;
        else if (window.getKey(glfw::KeyCode::Two))
            b = 1;
        else if (window.getKey(glfw::KeyCode::Three))
            b = 2;
        else if (window.getKey(glfw::KeyCode::Four))
            b = 3;
        else if (window.getKey(glfw::KeyCode::Five))
            b = 4;
        else if (window.getKey(glfw::KeyCode::Six))
            b = 5;
        else if (window.getKey(glfw::KeyCode::Seven))
            b = 6;
        else if (window.getKey(glfw::KeyCode::Eight))
            b = 7;
        else if (window.getKey(glfw::KeyCode::Nine))
            b = 8;
    }
}

void Window::update() {
    eventor->Update();
    world->update();
}

void Window::render() {
    renderer->render(world->get_draw_data());
}
