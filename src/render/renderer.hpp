//
// Created by Ninter6 on 2024/7/15.
//

#pragma once

#include "shader.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "render_type.hpp"
#include "oit.hpp"

#include "utils/event.h"
#include "world/world.hpp"

#include <span>
#include <memory>

class Window;

class Renderer {
public:
    Renderer(int w, int h);

    void init_event(Window& window, Eventor& eventor);

    void render(const DrawData& draw_data);
    void render_bcakground();
    void render_pass_3D(const DrawData& draw_data);
    void render_pass_2D(const DrawData& draw_data);

    [[nodiscard]] Camera* getCamera() const;

private:
    std::unique_ptr<Shader> simple;
    std::unique_ptr<Shader> special;
    std::unique_ptr<Shader> cube;
    std::unique_ptr<Shader> sky;
    std::unique_ptr<Shader> sun;
    std::unique_ptr<Shader> cloud;
    std::unique_ptr<Shader> screen;
    std::unique_ptr<Shader> ui;
    std::unique_ptr<Shader> composite;

    std::unique_ptr<VAO> cube_vao;
    std::unique_ptr<CubeVBO> cube_vbo;
    std::unique_ptr<QuadVBO> quad_vbo;
    std::unique_ptr<Buffer> ubo;

    bool main_mesh_update = false;
    int main_faces = 0;
    std::unique_ptr<Mesh> main_mesh;

    bool special_mesh_update = false;
    int special_faces = 0;
    std::unique_ptr<Mesh> special_mesh;

    struct ChunkData {
        ChunkPos pos;
        ChunkFace faces;
        int main_dirty = 0; // 0:clean, 1:modified, 2:buffer invalid
        int special_dirty = 0; // 0:clean, 1:modified, 2:buffer invalid
    };
    static constexpr int MAX_CHUNKS = (SIGHT_DISTANCE*2+1)*(SIGHT_DISTANCE*2+1);
    std::array<ChunkData, MAX_CHUNKS> chunks;

    int installed_chunks = 0;
    std::unordered_map<ChunkPos, int> chunk_map;

    std::unique_ptr<Texture> block_tex;
    std::unique_ptr<Texture> sky_tex;
    std::unique_ptr<Texture> star_tex;
    std::unique_ptr<Texture> sun_tex;
    std::unique_ptr<Texture> ui_tex;
    std::unique_ptr<Texture> cloud_tex;

    std::unique_ptr<OIT> oit;

    friend class Application;

private:
    void update_chunk(ChunkData& chunk, const ChunkFace& new_face);
    void update_chunks(const DrawData& data);
    void update_main_mesh(const std::pair<ChunkPos, ChunkPos>& visible_range);
    void update_special_mesh(const std::pair<ChunkPos, ChunkPos>& visible_range);
    void uninstall_useless_chunks();
    void update_ubo(const DrawData& data);

    void init_shader();
    void init_buffer();
    void init_chunk_buffers();
    void init_texture();
    void init_oit(int w, int h);

    void uninstall_chunk(int index);
    bool try_install_chunk(ChunkPos pos, const ChunkFace& face);
    void process_new_chunks(std::span<std::pair<ChunkPos, const ChunkFace*>> new_chunks, const std::pair<ChunkPos, ChunkPos>& visible_range);

    static bool is_chunk_visible(ChunkPos pos, const std::pair<ChunkPos, ChunkPos>& visible_range);

};
