//
// Created by Ninter6 on 2024/7/15.
//

#include "renderer.hpp"

#include "window.hpp"

#include "utils/check_gl.h"

Renderer::Renderer(int w, int h) {
    init_shader();
    init_camera(w, h);
    init_buffer();
    init_texture();
}

void Renderer::init_shader() {
    simple = std::make_unique<SimpleShader>();
    cube = std::make_unique<CubeShader>();
//    sky = std::make_unique<SkyShader>();
}

void Renderer::init_camera(int w, int h) {
    camera = std::make_unique<Camera>(mathpls::vec3{5, 64, 5}, mathpls::vec3{});
    camera->setProjPerspective(mathpls::radians(60.f), (float)w / (float)h, .1f, 100.f);
}

void Renderer::init_buffer() {
    cube_vao = std::make_unique<VAO>();
    cube_vao->bind();
    cube_vbo = std::make_unique<CubeVBO>();
    cube_vbo->bind_attrib();

    quad_vbo = std::make_unique<QuadVBO>();
    init_chunk_buffers();

    UBO ubo_data = {camera->proj, camera->view()};

    ubo = std::make_unique<Buffer>(GL_UNIFORM_BUFFER, true);
    ubo->buffer(&ubo_data, 0, sizeof(UBO));
    ubo->map();
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo->id);
}

void Renderer::init_chunk_buffers() {
    main_mesh = std::make_unique<Mesh>(true);
    main_mesh->bind();
    quad_vbo->bind_attrib();
    constexpr auto size = 16 * 16 * 128 * 3 * sizeof(Face) * MAX_CHUNKS;
    main_mesh->vbo.buffer(nullptr, 0, size);
    glEnableVertexAttribArray(2); // pos
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Face), nullptr);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(3); // facing
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(Face), (void*)offsetof(Face, facing));
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(4); // posOffset
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Face), (void*)offsetof(Face, posOffset));
    glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(5); // texIndex
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Face), (void*)offsetof(Face, texIndex));
    glVertexAttribDivisor(5, 1);
    glEnableVertexAttribArray(6); // sunIntensity
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Face), (void*)offsetof(Face, sunIntensity));
    glVertexAttribDivisor(6, 1);
    glEnableVertexAttribArray(7); // color
    glVertexAttribPointer(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Face), (void*)offsetof(Face, color));
    glVertexAttribDivisor(7, 1);
}

void Renderer::init_texture() {
    block_tex = Texture::LoadFromFile("/Users/mac/Desktop/temp/craftmine/res/images/blocks.png");
    block_tex->bind(0);

//    sky_transmittance = Texture::LoadFromFile("/Users/mac/Desktop/temp/craftmine/res/images/transmittance.jpg", true);
//    sky_transmittance->bind(1);
//    sky->set_texture("transmittance", 1);
}

void Renderer::init_event(Window& window, Eventor& eventor) {
    window.window.framebufferSizeEvent.setCallback([&](auto&& win, int w, int h) {
        glViewport(0, 0, w, h);
        camera->resetAspect((float)w / (float)h);
    });

    camera->init_event(eventor);
}

void Renderer::render(const DrawData& draw_data) {
    update_chunks(draw_data);
    update_main_mesh(draw_data.camera_visible_range);
    update_ubo(draw_data);

    render_pass_3D(draw_data);
}

void Renderer::render_bcakground() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


}

void Renderer::render_pass_3D(const DrawData& draw_data) {
    // sky pass
//    glDisable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_FRONT);
//
//    sky->use();
//    cube_vao->bind();
//    glDrawArrays(GL_TRIANGLES, 0, 36);

    // scene pass
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    simple->use();
    main_mesh->bind();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, main_faces);
    // I need this func to impl the further optimization: glDrawArraysInstancedBaseInstance
    // some platforms dont support it

//    auto [mn, mx] = draw_data.camera_visible_range;
//    std::printf("%d\n", (mx.x - mn.x + 16)*(mx.z - mn.z + 16)/256);

    // target block pass
    if (draw_data.camera_target_block) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glLineWidth(5.0f);
        mathpls::vec3 pos = *draw_data.camera_target_block;
        cube->use();
        glUniform3fv(glGetUniformLocation(cube->ID(), "position"), 1, pos.value_ptr());
        cube_vao->bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void Renderer::render_pass_2D() {

}

Camera* Renderer::getCamera() const{
    return camera.get();
}

void Renderer::update_chunk(Renderer::ChunkData& chunk, std::span<const Face> new_face) {
    if (chunk.faces.size() == new_face.size())
        chunk.dirty = 1;
    else
        chunk.dirty = 2;
    chunk.faces.clear();
    std::copy(new_face.begin(), new_face.end(), std::back_inserter(chunk.faces));
    main_mesh_update = true;
}

void Renderer::update_ubo(const DrawData& data) {
    UBO ubo_data{
        camera->proj,
        camera->view(),
        data.sun_dir,
        data.sun_I
    };
    std::memcpy(ubo->mem_map, &ubo_data, sizeof(ubo_data));
}

void Renderer::update_chunks(const DrawData& data) {
    std::vector<std::pair<ChunkPos, std::span<const Face>>> new_chunks;
    new_chunks.reserve(data.dirty_chunk.size() / 2);
    for (auto&& [pos, face] : data.dirty_chunk)
        if (!try_install_chunk(pos, face))
            new_chunks.emplace_back(pos, face);
    if (!new_chunks.empty())
        process_new_chunks(new_chunks, data.camera_visible_range);
}

bool Renderer::try_install_chunk(ChunkPos pos, std::span<const Face> face) {
    auto it = chunk_map.find(pos);
    if (it != chunk_map.end()) {
        update_chunk(chunks[it->second], face);
        return true;
    } else if (installed_chunks < MAX_CHUNKS) {
        chunks[installed_chunks].pos = pos;
        update_chunk(chunks[installed_chunks], face);
        chunk_map.emplace(pos, installed_chunks++);
        return true;
    }
    return false;
}

void Renderer::process_new_chunks(std::span<std::pair<ChunkPos, std::span<const Face>>> new_chunks, const std::pair<ChunkPos, ChunkPos>& visible_range) {
    std::vector<size_t> uninstall_chunks;
    uninstall_chunks.reserve(new_chunks.size());
    for (int i = MAX_CHUNKS - 1; uninstall_chunks.size() < new_chunks.size() && i >= 0; --i) {
        if (!is_chunk_visible(chunks[i].pos, visible_range))
            uninstall_chunks.push_back(i);
    }
    for (int i = 0; auto index : uninstall_chunks) {
        auto& pos = new_chunks[i].first;
        chunk_map.erase(chunks[index].pos);
        chunk_map.emplace(pos, index);
        chunks[index].pos = pos;
        update_chunk(chunks[index], new_chunks[i++].second);
    }
}

bool Renderer::is_chunk_visible(ChunkPos pos, const std::pair<ChunkPos, ChunkPos>& visible_range) {
    const auto& [min, max] = visible_range;
    return min.x <= pos.x && min.z <= pos.z && pos.x <= max.x && pos.z <= max.z;
}

void Renderer::update_main_mesh(const std::pair<ChunkPos, ChunkPos>& visible_range) {
    if (!main_mesh_update) return;
    main_mesh_update = false;
    main_mesh->bind();
    main_faces = 0;

    bool has_dirty = false;
    for (auto& c : chunks) {
        if (has_dirty && !is_chunk_visible(c.pos, visible_range)) continue;
        if (has_dirty || c.dirty) {
            main_mesh->vbo.subdata((void*) c.faces.data(),
                                   main_faces * sizeof(Face),
                                   c.faces.size() * sizeof(Face));
            if (c.dirty > 1) has_dirty = true;
            c.dirty = 0;
        }
        main_faces += (int)c.faces.size();
    }
}