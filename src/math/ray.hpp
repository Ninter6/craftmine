//
// Created by Ninter6 on 2024/7/22.
//

#pragma once

#include "mathpls.h"

struct Ray {
    mathpls::vec3 step() {
        auto near = floor(origin) - origin;
        float t = 1145e4;
        for (int i = 0; auto d : direction) {
            if (fabsf(d) < 1e-4f) { ++i; continue; }
            if (d > 0) near[i] += 1;
            t = fminf(t, near[i++] / d);
        }
        return floor(origin = at(t + 1e-4f));
    }

    [[nodiscard]] mathpls::vec3 at(float t) const {
        return origin + direction * t;
    }

    mathpls::vec3 origin;
    mathpls::vec3 direction;
};
