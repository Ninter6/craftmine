//
// Created by Ninter6 on 2024/8/5.
//

#include "ui.hpp"

#include <map>

basic_ui::basic_ui(mathpls::vec2 pos, mathpls::vec2 scale, float index)
: pos(pos), scale(scale), index(index) {}

std::vector<ui_unit> basic_ui::get_units() const {
    return {{.pos = pos, .scale = scale, .texIndex = index, .texture = 7}};
}

HotBar::HotBar(int length, float size, float y)
: bar(length), size(size), y(y) {
    constexpr BlockType B[] {
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
    std::copy_n(B, length, bar.begin());
}

float block_view(BlockType bt) {
    auto&& b = get_block(bt);
    if (b->special())
        return b->get_special_faces(1)[0]->firstTex;
    else
        return b->get_faces(1)[0]->texIndex;
}

std::vector<ui_unit> HotBar::get_units() const {
    std::vector<ui_unit> units;
    float left = -size * bar.size() / 2;
    for (size_t i = 0; i < bar.size(); ++i) {
        units.push_back({
            {left + i * size, y - size * .5f},
            size, 1, i == select ? 1.f : 0.f, 7
        });
        units.push_back({
            {left + (i + .2f) * size, y - size * .2f},
            size * 0.6f, 2, block_view(bar[i]), 0
        });
    }
    return units;
}

BlockType HotBar::get_selected_block() const {
    return bar[select];
}

void HotBar::init_event(Eventor& evt) {
    evt.add_event({0, 0, {}, 0, {}}, [&](auto&& lsn) {
        for (int i = 0; i < bar.size(); ++i)
            if (lsn.IsKeyDown('1' + i))
                select = i;
    });
}

#define MAX_UNITS 32

UIManager::UIManager(QuadVBO& quad) {
    ui_mesh = std::make_unique<Mesh>(true);
    ui_mesh->bind();
    quad.bind_attrib();
    ui_mesh->vbo.buffer(nullptr, 0, sizeof(ui_unit) * MAX_UNITS);
    glEnableVertexAttribArray(2); // pos
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ui_unit), nullptr);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(3); // scale
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ui_unit), (void*)offsetof(ui_unit, scale));
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(4); // priority
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ui_unit), (void*)offsetof(ui_unit, priority));
    glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(5); // texIndex
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ui_unit), (void*)offsetof(ui_unit, texIndex));
    glVertexAttribDivisor(5, 1);
}

size_t UIManager::add_widget(const std::shared_ptr<ui_widget>& widget) {
    widgets.push_back(widget);
    return widgets.size() - 1;
}

void UIManager::render_ui(Shader* sh) {
    if (widgets.empty()) return;

    std::map<int, std::vector<ui_unit>> map;
    for (auto& widget : widgets)
        for (auto& unit : widget->get_units())
            map[unit.texture].push_back(unit);

    for (auto&& [tex, units] : map) {
        ui_mesh->vbo.subdata(units.data(), 0, sizeof(ui_unit) * units.size());

        sh->use();
        sh->set_texture("tex", tex);

        ui_mesh->bind();
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, units.size());
    }
}
