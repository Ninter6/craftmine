//
// Created by Ninter6 on 2024/7/26.
//

#pragma once

#include "mathpls.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <numeric>

struct PNoise {
    explicit PNoise(uint32_t seed, float repeat = 0) : repeat(repeat) {
        std::iota(p, p + 256, 0);
        mathpls::random::xor_shift32 e{seed};
        for (int i = 256; i > 0; --i)
            std::swap(p[i-1], p[e() % i]);
        std::memcpy(p + 256, p, 256);
    }

    float operator()(float x, float y, float z) const {
        if(repeat > 0) {                                    // If we have any repeat on, change the coordinates to their "local" repetitions
            x = fmod(x, repeat);
            y = fmod(y, repeat);
            z = fmod(z, repeat);
        }

        int xi = (int)mathpls::floor(x) & 255;                              // Calculate the "unit cube" that the point asked will be located in
        int yi = (int)mathpls::floor(y) & 255;                              // The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
        int zi = (int)mathpls::floor(z) & 255;                              // plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
        float xf = mathpls::fract(x);
        float yf = mathpls::fract(y);
        float zf = mathpls::fract(z);

        float u = fade(xf);
        float v = fade(yf);
        float w = fade(zf);

        int aaa, aba, aab, abb, baa, bba, bab, bbb;
        aaa = p[p[p[    xi ]+    yi ]+    zi ];
        aba = p[p[p[    xi ]+inc(yi)]+    zi ];
        aab = p[p[p[    xi ]+    yi ]+inc(zi)];
        abb = p[p[p[    xi ]+inc(yi)]+inc(zi)];
        baa = p[p[p[inc(xi)]+    yi ]+    zi ];
        bba = p[p[p[inc(xi)]+inc(yi)]+    zi ];
        bab = p[p[p[inc(xi)]+    yi ]+inc(zi)];
        bbb = p[p[p[inc(xi)]+inc(yi)]+inc(zi)];

        float x1, x2, y1, y2;
        using std::lerp;
        x1 = lerp(    grad (aaa, xf  , yf  , zf),           // The gradient function calculates the dot product between a pseudorandom
                      grad (baa, xf-1, yf  , zf),             // gradient vector and the vector from the input coordinate to the 8
                      u);                                     // surrounding points in its unit cube.
        x2 = lerp(    grad (aba, xf  , yf-1, zf),           // This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
                      grad (bba, xf-1, yf-1, zf),             // values we made earlier.
                      u);
        y1 = lerp(x1, x2, v);

        x1 = lerp(    grad (aab, xf  , yf  , zf-1),
                      grad (bab, xf-1, yf  , zf-1),
                      u);
        x2 = lerp(    grad (abb, xf  , yf-1, zf-1),
                      grad (bbb, xf-1, yf-1, zf-1),
                      u);
        y2 = lerp (x1, x2, v);

        return lerp (y1, y2, w);
    }

    static constexpr float fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    [[nodiscard]] int inc(int n) const {
        return repeat > 0 ? (int)fmod((float)n + 1, repeat) : (n + 1);
    }
    static constexpr float grad(int hash, float x, float y, float z) {
        switch(hash & 0xF) {
            case 0x0: return  x + y; case 0x1: return -x + y; case 0x2: return  x - y; case 0x3: return -x - y;
            case 0x4: return  x + z; case 0x5: return -x + z; case 0x6: return  x - z; case 0x7: return -x - z;
            case 0x8: return  y + z; case 0x9: return -y + z; case 0xA: return  y - z; case 0xB: return -y - z;
            case 0xC: return  y + x; case 0xD: return -y + z; case 0xE: return  y - x; case 0xF: return -y - z;
            default: return 0; // never happens
        }
    }

    float repeat;
    uint8_t p[512]{};
};
