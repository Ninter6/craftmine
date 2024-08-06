//
// Created by Ninter6 on 2024/7/15.
//

#pragma once

#include <glad.h>
#include <glfwpp/glfwpp.h>

#include "math/mathpls.h"
#include "utils/event.h"

#include "renderer.hpp"
#include "ui.hpp"

struct WindowInfo {
    mathpls::ivec2 extent{};
    const char* title{};
};

class Window {
public:
    Window(const WindowInfo&);

    void loop();

private:
    glfw::Window window{};
    mathpls::ivec2 size{};

    std::unique_ptr<Renderer> renderer{};
    std::unique_ptr<UIManager> ui{};
    bool ui_visible = true;

    std::shared_ptr<HotBar> hotbar{};

    std::unique_ptr<Eventor> eventor{};

    std::unique_ptr<World> world{};

    friend Renderer;

private:
    void init_window(const WindowInfo& info);
    void init_render(int w, int h);
    void init_world();
    void init_event();

    void update();
    void render();

};
