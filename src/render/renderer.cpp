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
    oit = std::make_unique<OIT>(w, h);
}

void Renderer::init_shader() {
    simple = std::make_unique<SimpleShader>();
    special = std::make_unique<SpecialShader>();
    cube = std::make_unique<CubeShader>();
    sky = std::make_unique<SkyShader>();
    ui = std::make_unique<UIShader>();
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
    main_mesh->vbo.buffer(nullptr, 0, 16 * 16 * 128 * 3 * sizeof(Face) * MAX_CHUNKS);
    main_mesh->vbo.map();
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

    special_mesh = std::make_unique<Mesh>(true);
    special_mesh->bind();
    quad_vbo->bind_attrib();
    special_mesh->vbo.buffer(nullptr, 0, 16 * 16 * 32 * 3 * sizeof(SpecialFace) * MAX_CHUNKS);
    special_mesh->vbo.map();
    glEnableVertexAttribArray(2); // pos
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SpecialFace), nullptr);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(3); // facing
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(SpecialFace), (void*)offsetof(SpecialFace, facing));
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(4); // posOffset
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(SpecialFace), (void*)offsetof(SpecialFace, posOffset));
    glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(5); // sunIntensity
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(SpecialFace), (void*)offsetof(SpecialFace, sunIntensity));
    glVertexAttribDivisor(5, 1);
    glEnableVertexAttribArray(6); // color
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SpecialFace), (void*)offsetof(SpecialFace, color));
    glVertexAttribDivisor(6, 1);
    glEnableVertexAttribArray(7); // firstTex
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(SpecialFace), (void*)offsetof(SpecialFace, firstTex));
    glVertexAttribDivisor(7, 1);
    glEnableVertexAttribArray(8); // lastTex
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(SpecialFace), (void*)offsetof(SpecialFace, lastTex));
    glVertexAttribDivisor(8, 1);
    glEnableVertexAttribArray(9); // remainTick
    glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(SpecialFace), (void*)offsetof(SpecialFace, remainTick));
    glVertexAttribDivisor(9, 1);
}

void Renderer::init_texture() {
    block_tex = Texture::LoadFromFile("/Users/mac/Desktop/temp/craftmine/res/images/blocks.png");
    block_tex->bind(0);

    sky_tex = Texture::LoadFromFile("/Users/mac/Desktop/temp/craftmine/res/images/sky.png", true);
    sky_tex->bind(1);
    star_tex = Texture::LoadFromFile("/Users/mac/Desktop/temp/craftmine/res/images/star.png", true);
    star_tex->bind(2);
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
    update_special_mesh(draw_data.camera_visible_range);
    update_ubo(draw_data);
//    uninstall_useless_chunks();

    render_pass_3D(draw_data);
    render_pass_2D(draw_data);
}

void Renderer::render_bcakground() {
    // sky pass
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    sky->use();
    cube_vao->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Renderer::render_pass_3D(const DrawData& draw_data) {
    oit->opaque_pass();

    render_bcakground();

    // scene pass
    // normal face
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    simple->use();
    main_mesh->bind();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, main_faces);
    // I need this func to impl the further optimization: glDrawArraysInstancedBaseInstance
    // some platforms dont support it

    oit->transparent_pass();

    // special face
    special->use();
    special_mesh->bind();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, special_faces);

//    auto [mn, mx] = draw_data.camera_visible_range;
//    std::printf("%d\n", (mx.x - mn.x + 16)*(mx.z - mn.z + 16)/256);
}

void Renderer::render_pass_2D(const DrawData& draw_data) {
    // composite pass
    oit->composite(main_mesh->vao); // so that oit dosent need to create a vao

    // screen pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ui->use();
    oit->bind_final_tex(7);

    main_mesh->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // target block pass
    if (draw_data.camera_target_block) {
        mathpls::vec3 pos = *draw_data.camera_target_block;
        cube->use();
        glUniform3fv(glGetUniformLocation(cube->ID(), "position"), 1, pos.value_ptr());
        cube_vao->bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

Camera* Renderer::getCamera() const{
    return camera.get();
}

void Renderer::update_chunk(Renderer::ChunkData& chunk, const ChunkFace& new_face) {
    if (!new_face.normal_faces.empty() || !chunk.faces.normal_faces.empty()) {
        chunk.main_dirty = std::max(chunk.main_dirty,
                                    1 + (int) (chunk.faces.normal_faces.size() != new_face.normal_faces.size()));
        chunk.faces.normal_faces.clear();
        std::copy(new_face.normal_faces.begin(), new_face.normal_faces.end(), std::back_inserter(chunk.faces.normal_faces));
        main_mesh_update = true;
    }
    if (!new_face.special_faces.empty() || !chunk.faces.special_faces.empty()) {
        chunk.special_dirty = std::max(chunk.special_dirty,
                                       1 + (int) (chunk.faces.special_faces.size() != new_face.special_faces.size()));
        chunk.faces.special_faces.clear();
        std::copy(new_face.special_faces.begin(), new_face.special_faces.end(), std::back_inserter(chunk.faces.special_faces));
        special_mesh_update = true;
    }
}

void Renderer::update_ubo(const DrawData& data) {
    UBO ubo_data{
        camera->proj,
        camera->view(),
        data.sun_dir,
        data.sun_I
    };
    std::memcpy(ubo->mem_map, &ubo_data, sizeof(ubo_data));
    camera->is_dirty = false;
}

void Renderer::update_chunks(const DrawData& data) {
    std::vector<std::pair<ChunkPos, const ChunkFace*>> new_chunks;
    new_chunks.reserve(data.dirty_chunk.size() / 2);
    for (auto&& [pos, face] : data.dirty_chunk)
        if (!try_install_chunk(pos, face))
            new_chunks.emplace_back(pos, &face);
    if (!new_chunks.empty())
        process_new_chunks(new_chunks, data.camera_visible_range);
}

bool Renderer::try_install_chunk(ChunkPos pos, const ChunkFace& face) {
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

void Renderer::process_new_chunks(std::span<std::pair<ChunkPos, const ChunkFace*>> new_chunks, const std::pair<ChunkPos, ChunkPos>& visible_range) {
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
        update_chunk(chunks[index], *new_chunks[i++].second);
    }
}

bool Renderer::is_chunk_visible(ChunkPos pos, const std::pair<ChunkPos, ChunkPos>& visible_range) {
    const auto& [min, max] = visible_range;
    return min.x <= pos.x && min.z <= pos.z && pos.x <= max.x && pos.z <= max.z;
}

void Renderer::uninstall_chunk(int index) {
    installed_chunks--;
    if(index == installed_chunks) {
        chunk_map.erase(chunks[index].pos);
        return;
    }
    chunk_map.erase(chunks[index].pos);
    chunk_map[chunks[installed_chunks].pos] = index;
    std::swap(chunks[index], chunks[installed_chunks]);
}

void Renderer::uninstall_useless_chunks() {
    for (int i = installed_chunks - 1; i >= 0; --i)
        if ((chunks[i].main_dirty == 2 || chunks[i].faces.normal_faces.empty()) &&
            (chunks[i].special_dirty == 2 || chunks[i].faces.special_faces.empty()))
            uninstall_chunk(i);
}

void Renderer::update_main_mesh(const std::pair<ChunkPos, ChunkPos>& visible_range) {
    if (!main_mesh_update) return;
    main_mesh_update = false;
    main_mesh->bind();
    main_faces = 0;

    int i = 0;
    bool has_dirty = false;
    for (auto& c : chunks) {
        if (i++ >= installed_chunks) break;
        if (has_dirty && !is_chunk_visible(c.pos, visible_range)) {
            c.main_dirty = 2;
            continue;
        }
        if (has_dirty || c.main_dirty) {
            std::memcpy((Face*) main_mesh->vbo.mem_map + main_faces,
                        c.faces.normal_faces.data(),
                        c.faces.normal_faces.size() * sizeof(Face));
            if (c.main_dirty > 1) has_dirty = true;
            c.main_dirty = 0;
        }
        main_faces += (int)c.faces.normal_faces.size();
    }
}

void Renderer::update_special_mesh(const std::pair<ChunkPos, ChunkPos>& visible_range) {
    if (!special_mesh_update) return;

    special_mesh_update = false;
    special_mesh->bind();
    special_faces = 0;

    int i = 0;
    bool has_dirty = false;
    for (auto& c : chunks) {
        if (i++ >= installed_chunks) break;
        if (has_dirty && !is_chunk_visible(c.pos, visible_range)) {
            c.special_dirty = 2;
            continue;
        }
        if (has_dirty || c.special_dirty) {
            std::memcpy((SpecialFace*) special_mesh->vbo.mem_map + special_faces,
                        c.faces.special_faces.data(),
                        c.faces.special_faces.size() * sizeof(SpecialFace));
            if (c.special_dirty > 1) has_dirty = true;
            c.special_dirty = 0;
        }
        special_faces += (int)c.faces.special_faces.size();
    }
}