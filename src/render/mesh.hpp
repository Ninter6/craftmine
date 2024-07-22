//
// Created by Ninter6 on 2024/7/15.
//

#pragma once

#include "glad.h"

#include <cstdlib>

struct VAO {
    VAO();
    ~VAO();

    VAO(VAO&&) = delete;

    void bind() const;

    GLuint id;
};

struct Buffer {
    Buffer(GLint type, bool dynamic);
    ~Buffer();

    Buffer(Buffer&&) = delete;

    void bind() const;
    void buffer(void *data, size_t offset, size_t size);
    void subdata(void *data, size_t offset, size_t length) const;

    void* map() const;
    void unmap() const;

    size_t size;

    GLint type;
    GLuint id;
    bool dynamic;
};

struct QuadVBO : Buffer {
    QuadVBO();
    void bind_attrib() const;
};

struct CubeVBO : Buffer {
    CubeVBO();
    void bind_attrib() const;
};

struct Mesh {
    Mesh(bool dynamic);

    void bind() const;

    VAO vao;
    Buffer vbo;
};