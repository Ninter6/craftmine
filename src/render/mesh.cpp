//
// Created by Ninter6 on 2024/7/15.
//

#include "mesh.hpp"

VAO::VAO() {
    glGenVertexArrays(1, &id);
}

VAO::~VAO() {
    glDeleteVertexArrays(1, &id);
}

void VAO::bind() const {
    glBindVertexArray(id);
}

Buffer::Buffer(GLint type, bool dynamic) : type(type), dynamic(dynamic) {
    glGenBuffers(1, &id);
}

Buffer::~Buffer() {
    glDeleteBuffers(1, &id);
}

void Buffer::bind() const {
    glBindBuffer(type, id);
}

void Buffer::buffer(void *data, size_t offset, size_t _size) {
    this->size = _size;
    bind();
    glBufferData(type, (GLintptr)(_size - offset), data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void Buffer::subdata(void *data, size_t offset, size_t length) const {
    bind();
    glBufferSubData(type, (GLintptr)offset, (GLintptr)length, data);
}

void* Buffer::map() const {
    return glMapBuffer(type, GL_WRITE_ONLY);
}

void Buffer::unmap() const {
    glUnmapBuffer(type);
}

QuadVBO::QuadVBO() : Buffer(GL_ARRAY_BUFFER, false) {
    float v[] = { 0,0, 1,1, 1,0, 0,1, 1,1, 0,0 };
    bind();
    buffer(v, 0, sizeof(v));
}

void QuadVBO::bind_attrib() const {
    bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
}

CubeVBO::CubeVBO() : Buffer(GL_ARRAY_BUFFER, false) {
    float vertices[] = {
        // positions       // normals           // uv
        1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        0.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        0.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        1.0f, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

        0.0f, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        1.0f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.0f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        1.0f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        0.0f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        1.0f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
    bind();
    buffer(vertices, 0, sizeof(vertices));
}

void CubeVBO::bind_attrib() const {
    bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
}

Mesh::Mesh(bool dynamic) : vao{}, vbo{GL_ARRAY_BUFFER, dynamic} {}

void Mesh::bind() const {
    vao.bind();
}