//
// Created by Ninter6 on 2024/7/15.
//

#include "window.hpp"

#include "world/world.hpp"

#include <iostream>

auto GLFW = glfw::init();

Window::Window(const WindowInfo& info) : size(info.extent) {
    init_window(info);
    init_render(size.x, size.y);
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

    hotbar = std::make_unique<HotBar>(9);
    ui = std::make_unique<UIManager>(*renderer->quad_vbo);
    ui->add_widget(hotbar);
    ui->add_widget(std::shared_ptr<ui_widget>{new basic_ui{-.05f, .1f, 2}});
}

void Window::init_world() {
    uint32_t c, seed = 114514;
    std::string file, name;
    std::cout << "Type to choose:\n" "0. load world\n" "1. new world\n> ";
    std::cin >> c;
    if (c) {
        std::cout << "Type your world's name:\n> ";
        std::cin >> name;
        std::cout << "Type seed:\n> ";
        std::cin >> seed;
    } else {
        std::cout << "Type your world's name or full path to your file:\n> ";
        std::cin >> file;
    }
    world = std::make_unique<World>(WorldInitInfo{
        .file = c ? "" : file.ends_with(".cmw") ? std::move(file) : worldname2filename(file),
        .name = name.empty() ? "myworld" : std::move(name),
        .seed = seed,
        .camera = renderer->getCamera()
    });
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
    eventor->add_event({0, 0, 'V', 0}, [&](auto&& lsn) {
        if (!lsn.IsKeyPressed('V')) return;
        ui_visible = !ui_visible;
    });
    eventor->add_event({4, 0, 'S', 0}, [&](auto&& lsn) {
        world->save();
    });

    renderer->init_event(*this, *eventor);

    hotbar->init_event(*eventor);

    eventor->add_event({{}, {}, {}, 0, {}}, [&](auto&& lsn) {
        if (lsn.IsMouseButtonReleased(1))
            world->set_camera_target_block(BlockType::air, false);
        else if (lsn.IsMouseButtonReleased(2))
            world->set_camera_target_block(hotbar->get_selected_block(), true);
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
    }
}

void Window::update() {
    eventor->Update();
    world->update();
}

void Window::render() {
    renderer->render(world->get_draw_data());
    if (ui_visible) ui->render_ui(renderer->ui.get());
}
