//
// Created by Ninter6 on 2024/7/26.
//

#pragma once

#include "math/pnoise.h"

struct NoiseFactory {
    virtual ~NoiseFactory() = default;
    virtual float compute(PNoise& noise, float x, float z) = 0;
};

struct Basic : NoiseFactory {
    Basic(float o) : o(o) {}

    float o;

    float compute(PNoise& noise, float x, float z) override {
        return noise(x, z, o * 32);
    }
};

struct Octave : NoiseFactory {
    Octave(int n, float o) : n(n), o(o) {}

    int n;
    float o;

    float compute(PNoise& noise, float x, float z) override {
        float u = 1.f, v = 0.f;
        for (int i = 0; i < n; i++) {
            v += (1.f / u) * noise((x / 1.01f) * u, (z / 1.01f) * u, o * 32);
            u *= 2.f;
        }
        return v;
    }
};

struct Combined : NoiseFactory {
    Combined(NoiseFactory* n, NoiseFactory* m) : n(n), m(m) {}

    NoiseFactory *n, *m;

    float compute(PNoise& noise, float x, float z) override {
        return n->compute(noise, x + m->compute(noise, x, z), z);
    }
};

struct ExpScale : NoiseFactory {
    ExpScale(NoiseFactory* n, float exp, float scale) : n(n), exp(exp), scale(scale) {}

    NoiseFactory* n;
    float exp, scale;

    float compute(PNoise& noise, float x, float z) override {
        float v = n->compute(noise, x * scale, z * scale);
        return (0 < v ? 1.f : -1.f) * std::pow(std::abs(v), exp);
    }
};
