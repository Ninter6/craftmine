//
// Created by Ninter6 on 2024/8/5.
//

#pragma once

#include <vector>

#include "math/mathpls.h"
#include "utils/event.h"

#include "block/block.hpp"

#include "mesh.hpp"
#include "shader.hpp"

struct ui_unit {
    mathpls::vec2 pos; // [-1, 1]
    mathpls::vec2 scale = 1;
    float priority = 1; // [1, ...]
    float texIndex;
    int texture; // 0: block, 7: ui
};

struct ui_widget {
    virtual ~ui_widget() = default;
    virtual std::vector<ui_unit> get_units() const = 0;
};

struct basic_ui : ui_widget {
    basic_ui(mathpls::vec2 pos, mathpls::vec2 scale, float index);
    std::vector<ui_unit> get_units() const override;

    mathpls::vec2 pos; // [-1, 1]
    mathpls::vec2 scale;
    float index;
};

struct HotBar : ui_widget {
    HotBar(int length, float size = .1f, float y = -.8f);

    std::vector<ui_unit> get_units() const override;

    BlockType get_selected_block() const;

    void init_event(Eventor& evt);

    float size;
    float y;

    int select = 0;
    std::vector<BlockType> bar;
};

class UIManager {
public:
    UIManager(QuadVBO& quad);

    size_t add_widget(const std::shared_ptr<ui_widget>& widget);

    void render_ui(Shader* sh);

private:
    std::vector<std::shared_ptr<ui_widget>> widgets;

    std::unique_ptr<Mesh> ui_mesh;

};
