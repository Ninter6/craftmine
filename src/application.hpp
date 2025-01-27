//
// Created by Ninter6 on 2025/1/27.
//

#pragma once

#include "render/window.hpp"
#include "render/renderer.hpp"
#include "render/ui.hpp"

class Application {
public:
    Application() = default;

    void init();
    void run();
    void destroy();

    Window* window;
    Eventor* eventor;
    Renderer* renderer;
    UIManager* ui;
    bool ui_visible = true;

    std::shared_ptr<HotBar> hotbar;

    std::shared_ptr<Camera> active_camera;
    std::shared_ptr<World> active_world;
    std::shared_ptr<BlockMaterial> block_material;

private:
    void init_camera();
    void init_world();
    void init_window();
    void init_render();
    void init_event();

    void update();
    void render();

};

extern Application* app;

World* get_active_world();
Camera* get_active_camera();
BlockMaterial* get_block_material();
