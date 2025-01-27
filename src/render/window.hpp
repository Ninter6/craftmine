//
// Created by Ninter6 on 2024/7/15.
//

#pragma once

#include <glad.h>
#include <glfwpp/glfwpp.h>

#include "math/mathpls.h"
#include "utils/event.h"

struct WindowInfo {
    mathpls::ivec2 extent{};
    const char* title{};
};

class Window {
public:
    Window(const WindowInfo&);

    bool should_close();
    void next_frame();

private:
    glfw::Window window{};
    mathpls::ivec2 size{};

    std::unique_ptr<Eventor> eventor{};

    friend class Renderer;
    friend class Application;

private:
    void init_window(const WindowInfo& info);
    void init_event();

};
