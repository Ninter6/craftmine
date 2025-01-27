//
// Created by Ninter6 on 2025/1/27.
//

#include "application.hpp"

#include <iostream>

Application* app = nullptr;

constexpr mathpls::ivec2 win_ext{1600, 1000};

void Application::init() {
    init_camera();
    init_world();
    init_window();
    init_render();
    init_event();
}

void Application::init_camera() {
    active_camera = std::make_shared<Camera>(mathpls::vec3{5, 64, 5}, mathpls::vec3{});
    active_camera->setProjPerspective(mathpls::radians(60.f), (float)win_ext.x / (float)win_ext.y, .1f, 100.f);
}

void check_material(BlockMaterial* mat) {
    std::cout << "block count:" << mat->table.size() << '\n';
    for (int i = 0; auto&& [name, block] : mat->table) {
        std::cout << '[' << i++ << "]: " << name << '\n';
    }
}

void Application::init_world() {
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

    std::cout << "Type path to your material file (empty to use default):\n> ";
    std::string matfile;
    std::cin.ignore(1, '\n');
    std::getline(std::cin, matfile);
    if (matfile.empty()) matfile = FILE_ROOT"materials/craftmine.txt";
    if (matfile[0] != '/') matfile = FILE_ROOT + matfile;
    block_material = std::make_shared<BlockMaterial>(matfile);
    check_material(block_material.get());

    active_world = std::make_unique<World>(WorldInitInfo{
        .file = c ? "" : file.ends_with(".cmw") ? std::move(file) : worldname2filename(file),
        .name = name.empty() ? "myworld" : std::move(name),
        .seed = seed
    });
}

void Application::init_window() {
    window = new Window{{
        .extent = win_ext,
        .title = "craftmine"
    }};
}

void Application::init_render() {
    renderer = new Renderer(win_ext.x, win_ext.y);

    hotbar = std::make_shared<HotBar>(9);
    ui = new UIManager(*renderer->quad_vbo);
    ui->add_widget(hotbar);
    ui->add_widget(std::shared_ptr<ui_widget>{new basic_ui{-.05f, .1f, 2}});
}

void Application::init_event() {
    eventor = window->eventor.get();

    eventor->add_event({0, 0, 'V', 0}, [&](auto&& lsn) {
        if (!lsn.IsKeyPressed('V')) return;
        ui_visible = !ui_visible;
    });
    eventor->add_event({4, 0, 'S', 0}, [&](auto&& lsn) {
        active_world->save();
    });

    renderer->init_event(*window, *eventor);

    hotbar->init_event(*eventor);

    eventor->add_event({{}, {}, {}, 0, {}}, [&](auto&& lsn) {
        if (lsn.IsMouseButtonReleased(1))
            active_world->set_camera_target_block(BlockType::air, false);
        else if (lsn.IsMouseButtonReleased(2))
            active_world->set_camera_target_block(hotbar->get_selected_block(), true);
    });
}

void Application::run() {
    using cl = std::chrono::high_resolution_clock;
    auto t = cl::now();
    int count = 0;
    while (!window->should_close()) {
        update();
        render();

        window->next_frame();

        count++;
        if (cl::now() - t >= std::chrono::seconds{1}) {
            std::printf("\rFPS: %d", count);
            t = cl::now();
            count = 0;
        }
    }
}

void Application::update() {
    eventor->Update();
    active_world->update();
}

void Application::render() {
    renderer->render(active_world->get_draw_data());
    if (ui_visible) ui->render_ui(renderer->ui.get());
}


void Application::destroy() {
    delete window;
    delete renderer;
    delete ui;
}

World* get_active_world() {
    return app->active_world.get();
}

Camera* get_active_camera() {
    return app->active_camera.get();
}

BlockMaterial* get_block_material() {
    return app->block_material.get();
}

