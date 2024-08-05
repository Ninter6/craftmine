//
// Created by Ninter6 on 2024/8/3.
//

#pragma once

#include <glad.h>

#include <memory>

#include "shader.hpp"
#include "mesh.hpp"

class OIT {
public:
    OIT(int w, int h, Shader* comp);
    ~OIT();

    OIT(OIT&&) = delete;

    void bind_fbo();
    void opaque_pass();
    void transparent_pass();
    void composite(VAO& quad);
    void bind_final_tex(int n);

private:
    GLuint fbo;
    GLuint opaque, accumulation, revealage, depth;

    Shader* comp;
};
