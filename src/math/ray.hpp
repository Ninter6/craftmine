//
// Created by Ninter6 on 2024/7/22.
//

#pragma once

#include "mathpls.h"

struct Ray {
    mathpls::vec3 step() {
        auto dis = mathpls::vec3{
            direction.x > 0 ? 1 - mathpls::fract(origin.x) : -mathpls::fract(origin.x),
            direction.y > 0 ? 1 - mathpls::fract(origin.y) : -mathpls::fract(origin.y),
            direction.z > 0 ? 1 - mathpls::fract(origin.z) : -mathpls::fract(origin.z)
        };
        for (int i = 0; i < 3; ++i)
            if (direction[i] == 0) dis[i] = 9e9;
            else dis[i] /= direction[i];
        auto mt = mathpls::min(dis.x, mathpls::min(dis.y, dis.z));
        origin = at(mt + 1e-5f);
        return mathpls::floor(origin);
    }

    mathpls::vec3 at(float t) const {
        return origin + direction * t;
    }

    mathpls::vec3 origin;
    mathpls::vec3 direction;
};
