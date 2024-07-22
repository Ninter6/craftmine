//
// Created by Ninter6 on 2024/7/15.
//

#pragma once

#include "shader.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "render_type.hpp"

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
    void render_pass_2D();

    [[nodiscard]] Camera* getCamera() const;

private:
    std::unique_ptr<Shader> simple;
    std::unique_ptr<Shader> cube;

    std::unique_ptr<VAO> cube_vao;
    std::unique_ptr<CubeVBO> cube_vbo;
    std::unique_ptr<QuadVBO> quad_vbo;
    std::unique_ptr<Buffer> cam_ubo;

    static constexpr int MAX_CHUNKS = (SIGHT_DISTANCE*2+1)*(SIGHT_DISTANCE*2+1);
    std::array<std::unique_ptr<Mesh>, MAX_CHUNKS> chunks;

    struct ChunkInfo {
        int index{};
        int size{};
    };
    int installed_chunks = 0;
    std::unordered_map<ChunkPos, ChunkInfo> chunk_map;

    std::unique_ptr<Camera> camera;

    std::unique_ptr<Texture> block_tex;

private:
    static void update_chunk(Buffer& chunk, std::span<const Face> face);
    void update_chunks(const DrawData& data);
    void update_camera_ubo();

    void init_shader();
    void init_camera(int w, int h);
    void init_buffer();
    void init_chunk_buffers();
    void init_texture();

    bool try_install_chunk(ChunkPos pos, std::span<const Face> face);
    void process_new_chunks(std::span<std::pair<ChunkPos, std::span<const Face>>> new_chunks, const std::pair<ChunkPos, ChunkPos>& visible_range);

    static bool is_chunk_visible(ChunkPos pos, const std::pair<ChunkPos, ChunkPos>& visible_range);

};
