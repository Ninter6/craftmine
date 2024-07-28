//
// Created by Ninter6 on 2024/7/27.
//

#include "decoration.hpp"

#include <world/chunk.hpp>
#include <world/world.hpp>

static constexpr size_t hash(int a, int b, int c, int d, int e) {
    size_t r = 0;
    r ^= std::hash<int>{}(a) + (r << 6) ^ (r >> 2);
    r ^= std::hash<int>{}(b) + (r << 6) ^ (r >> 2);
    r ^= std::hash<int>{}(c) + (r << 6) ^ (r >> 2);
    r ^= std::hash<int>{}(d) + (r << 6) ^ (r >> 2);
    r ^= std::hash<int>{}(e) + (r << 6) ^ (r >> 2);
    return r;
}

#define SET(w, c, x_, y_, z_, t) do { \
    if (!(x_ & ~15) && !(z_ & ~15)) \
        c.set_block({x_, y_, z_}, t); \
    else                           \
        w->set_block({c.position.x + x_, y_, c.position.z + z_}, t); \
} while (0)

bool Oak::apply(World* w, Chunk& c, int seed, int x, int y, int z) const {
    auto under = c.get_block({x, y-1, z});
    if (under != BlockType::grass_block && under != BlockType::dirt)
        return false;

    std::mt19937_64 e(hash(seed, x, y, z, 114514));
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    if (dis(e) > chance)
        return false;

    if (under == BlockType::grass_block)
        c.set_block({x, y-1, z}, BlockType::dirt);

    const auto& pos = c.position;

    int h = 3 + (e() % 3);
    for (int i = 0; i < h; ++i)
        c.set_block({x, y + i, z}, BlockType::log);

    int lh = 2 + (e() % 2);

    for (int xx = (x - 2); xx <= (x + 2); xx++) {
        for (int zz = (z - 2); zz <= (z + 2); zz++) {
            for (int yy = (y + h - 1); yy <= y + h; yy++) {
                int n = 0;
                n += xx == (x - 2) || xx == (x + 2);
                n += zz == (z - 2) || zz == (z + 2);
                bool corner = n == 2;

                if ((!(xx == x && zz == z) || yy > (y + h)) &&
                    !(corner && yy == (y + h + 1) && dis(e) < 0.4)) {
                    SET(w, c, xx, yy, zz, BlockType::leaf);
                }
            }
        }
    }
    for (int xx = (x - 1); xx <= (x + 1); xx++) {
        for (int zz = (z - 1); zz <= (z + 1); zz++) {
            for (int yy = (y + h + 1); yy < (y + h + lh); yy++) {
                int n = 0;
                n += xx == (x - 1) || xx == (x + 1);
                n += zz == (z - 1) || zz == (z + 1);
                bool corner = n == 2;

                if (!(corner && yy == (y + h + lh) && dis(e) < 0.8)) {
                    SET(w, c, xx, yy, zz, BlockType::leaf);
                }
            }
        }
    }
    if (dis(e) < 0.3) c.set_block({x, y + h + lh, z}, BlockType::leaf);
    return true;
}

bool Pine::apply(World* w, Chunk& c, int seed, int x, int y, int z) const {
    auto under = c.get_block({x, y-1, z});
    if (under != BlockType::grass_block && under != BlockType::dirt &&
        under != BlockType::snow && under != BlockType::podzol)
        return false;

    std::mt19937_64 e(hash(seed, x, y, z, 1919810));
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    if (dis(e) > chance)
        return false;

    if (under == BlockType::grass_block || under == BlockType::podzol)
        c.set_block({x, y-1, z}, BlockType::dirt);


    int h = 5 + (e() % 3);
    for (int i = 0; i < h; ++i)
        c.set_block({x, y + i, z}, BlockType::pine_log);

    int r = 2 + (e() % 2);

    for(int yy = (y + 2); yy < (y + h); yy++) {
        if (yy != (y + h - 1) && yy % 2 == 0)
            continue;

        if (yy == (y + h - 1))
            r = 1;
        else if (r > 1 && dis(e) < 0.7)
            r--;

        for (int xx = (x - r); xx <= (x + r); xx++) {
            for (int zz = (z - r); zz <= (z + r); zz++) {
                int n = 0;
                n += xx == (x - r) || xx == (x + r);
                n += zz == (z - r) || zz == (z + r);
                bool corner = n == 2;

                if (!corner && !(xx == x && zz == z)) {
                    SET(w, c, xx, yy, zz, BlockType::pine_leaf);
                }
            }
        }
    }
    c.set_block({x, y+h, z}, BlockType::pine_leaf);
    return true;
}

bool Flower::apply(World* w, Chunk& c, int seed, int x, int y, int z) const {
    std::mt19937_64 e(hash(seed, x, y, z, 1149999));
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    if (dis(e) > chance)
        return false;

    auto flower = dis(e) > 0.6 ? BlockType::rose : BlockType::dandelion;

    int s = 2 + (e() % 5);
    int l = s - 1 + (e() % 3);
    int h = s - 1 + (e() % 3);

    using mathpls::clamp;
    for (int xx = clamp(x - l, 0, 15); xx <= clamp(x + l, 0, 15); xx++) {
        for (int zz = clamp(z - h, 0, 15); zz <= clamp(z + h, 0, 15); zz++) {
            auto block = c.get_block({xx, y, zz}),
                 under = c.get_block({xx, y-1, zz});
            if (block == BlockType::air &&
                under == BlockType::grass_block &&
                dis(e) < 0.314f) {
                SET(w, c, xx, y, zz, flower);
            }
        }
    }
    return true;
}

bool Grass::apply(World* w, Chunk& c, int seed, int x, int y, int z) const {
    std::mt19937_64 e(hash(seed, x, y, z, 8881919));
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    if (dis(e) > chance)
        return false;

    int s = 2 + (e() % 5);
    int l = s - 1 + (e() % 3);
    int h = s - 1 + (e() % 3);

    using mathpls::clamp;
    for (int xx = clamp(x - l, 0, 15); xx <= clamp(x + l, 0, 15); xx++) {
        for (int zz = clamp(z - h, 0, 15); zz <= clamp(z + h, 0, 15); zz++) {
            auto block = c.get_block({xx, y, zz}),
                under = c.get_block({xx, y-1, zz});
            if (block == BlockType::air &&
                under == BlockType::grass_block &&
                dis(e) < 0.314f) {
                SET(w, c, xx, y, zz, BlockType::grass);
            }
        }
    }
    return true;
}

bool Shrub::apply(World* w, Chunk& c, int seed, int x, int y, int z) const {
    auto under = c.get_block({x, y-1, z});
    if (under != BlockType::sand && under != BlockType::grass_block && under != BlockType::dirt)
        return false;

    std::mt19937_64 e(hash(seed, x, y, z, 114514));
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    if (dis(e) > chance)
        return false;

    SET(w, c, x, y, z, BlockType::shrub);
    return true;
}