//
// Created by Ninter6 on 2024/7/15.
//

#pragma once

#include "math/mathpls.h"
#include "utils/event.h"

struct Frustum {
    float fovy, asp;
    float near, far;
};

struct Camera {
    Camera() = default;
    Camera(mathpls::vec3 pos, mathpls::vec3 target);

    void init_event(Eventor& e);

    void setProjPerspective(float fovy, float aspect, float near, float far);
    void setProjOrtho(mathpls::vec2 min, mathpls::vec2 max);

    void resetAspect(float aspect);

    [[nodiscard]] mathpls::mat4 view() const;
    [[nodiscard]] mathpls::mat4 projView() const;

    mathpls::vec3 position;
    mathpls::vec3 forward;

    mathpls::mat4 proj;
    Frustum frustum;
};
