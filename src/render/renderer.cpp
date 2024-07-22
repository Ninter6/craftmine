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
}

void Renderer::init_camera(int w, int h) {
    camera = std::make_unique<Camera>(mathpls::vec3{5, 16, 5}, mathpls::vec3{});
    camera->setProjPerspective(mathpls::radians(60.f), (float)w / (float)h, .1f, 100.f);
}

void Renderer::init_buffer() {
    cube_vao = std::make_unique<VAO>();
    cube_vao->bind();
    cube_vbo = std::make_unique<CubeVBO>();
    cube_vbo->bind_attrib();

    quad_vbo = std::make_unique<QuadVBO>();
    init_chunk_buffers();

    CameraUBO ubo_data = {camera->proj, camera->view()};

    cam_ubo = std::make_unique<Buffer>(GL_UNIFORM_BUFFER, false);
    cam_ubo->buffer(&ubo_data, 0, sizeof(CameraUBO));
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cam_ubo->id);
}

void Renderer::init_chunk_buffers() {
    for (auto&& c : chunks) {
        c = std::make_unique<Mesh>(true);
        c->bind();
        quad_vbo->bind_attrib();
        c->vbo.buffer(nullptr, 0, 16 * 16 * 256 * 3 * sizeof(Face));
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
        glEnableVertexAttribArray(6); // lightIntensity
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Face), (void*)offsetof(Face, lightIntensity));
        glVertexAttribDivisor(6, 1);
        glEnableVertexAttribArray(7); // color
        glVertexAttribPointer(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Face), (void*)offsetof(Face, color));
        glVertexAttribDivisor(7, 1);
    }
}

void Renderer::init_texture() {
    block_tex = Texture::LoadFromFile("/Users/mac/Desktop/temp/craftmine/res/images/blocks.png");
    block_tex->bind();
}

void Renderer::init_event(Window& window, Eventor& eventor) {
    window.window.framebufferSizeEvent.setCallback([&](auto&& win, int w, int h) {
        glViewport(0, 0, w, h);
        camera->resetAspect((float)w / (float)h);
        camera->is_dirty = true;
    });

    Event evt{};

    evt.MouseMove = true;
    eventor.add_event(evt, [&](double x, double y) {
        auto dt = mathpls::radians(x);
        auto& [x0, y0, z0] = camera->forward.asArray;
        x0 = cos(dt)*x0 - sin(dt)*z0;
        z0 = sin(dt)*x0 + cos(dt)*z0;
        y0 -= y * .02;
        camera->forward.normalize();
        camera->is_dirty = true;
    });

    evt.MouseMove = std::nullopt;
    evt.NormalKey = 'F';
    eventor.add_event(evt, [&]() {
        camera->forward = {0, 0, 1};
        camera->is_dirty = true;
    });

    evt.NormalKey = 'W';
    eventor.add_event(evt, [&]() {
        camera->position += camera->forward * .1f;
        camera->is_dirty = true;
    });
    evt.NormalKey = 'S';
    eventor.add_event(evt, [&]() {
        camera->position -= camera->forward * .1f;
        camera->is_dirty = true;
    });
    evt.NormalKey = 'A';
    eventor.add_event(evt, [&]() {
        camera->position -= mathpls::cross(camera->forward, {0, 1, 0}).normalized() * .1f;
        camera->is_dirty = true;
    });
    evt.NormalKey = 'D';
    eventor.add_event(evt, [&]() {
        camera->position += mathpls::cross(camera->forward, {0, 1, 0}).normalized() * .1f;
        camera->is_dirty = true;
    });
}

void Renderer::render(const DrawData& draw_data) {
    render_pass_3D(draw_data);
}

void Renderer::render_bcakground() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


}

void Renderer::render_pass_3D(const DrawData& draw_data) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    update_chunks(draw_data);
    update_camera_ubo();
    simple->use();

    for (auto&& [pos, info] : chunk_map) {
        assert(!(pos.x%16 || pos.z%16));
        chunks[info.index]->bind();
        if (is_chunk_visible(pos, draw_data.camera_visible_range))
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, info.size);
    }

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

void Renderer::update_chunk(Buffer& chunk, std::span<const Face> face) {
    auto data = (void*)face.data();
    chunk.subdata(data, 0, face.size() * sizeof(Face));
}

void Renderer::update_camera_ubo() {
    if (!camera->is_dirty) return;
    CameraUBO ubo_data = {camera->proj, camera->view()};
    cam_ubo->subdata(&ubo_data, 0, sizeof(CameraUBO));
    camera->is_dirty = false;
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
        update_chunk(chunks[it->second.index]->vbo, face);
        it->second.size = (int)face.size();
        return true;
    } else if (installed_chunks < MAX_CHUNKS) {
        update_chunk(chunks[installed_chunks]->vbo, face);
        chunk_map.emplace(pos, ChunkInfo{installed_chunks++, (int)face.size()});
        return true;
    }
    return false;
}

void Renderer::process_new_chunks(std::span<std::pair<ChunkPos, std::span<const Face>>> new_chunks, const std::pair<ChunkPos, ChunkPos>& visible_range) {
    std::vector<decltype(chunk_map.end())> uninstall_chunks;
    uninstall_chunks.reserve(new_chunks.size());
    for (auto it = chunk_map.begin(); uninstall_chunks.size() <= new_chunks.size() && it != chunk_map.end(); ++it) {
        if (!is_chunk_visible(it->first, visible_range))
            uninstall_chunks.push_back(it);
    }
    for (int i = 0; auto it : uninstall_chunks) {
        auto index = it->second.index;
        update_chunk(chunks[index]->vbo, new_chunks[i].second);
        chunk_map.erase(it);
        chunk_map.emplace(new_chunks[i].first, ChunkInfo{index, (int)new_chunks[i].second.size()});
        i++;
    }
}

bool Renderer::is_chunk_visible(ChunkPos pos, const std::pair<ChunkPos, ChunkPos>& visible_range) {
    const auto& [min, max] = visible_range;
    return min.x <= pos.x && min.z <= pos.z && pos.x <= max.x && pos.z <= max.z;
}