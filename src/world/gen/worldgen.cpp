//
// Created by Ninter6 on 2024/7/26.
//

#include <cassert>

#include "worldgen.hpp"
#include "biome.hpp"

#define SEA_LEVEL 36

WorldGen::WorldGen(uint32_t seed) : noise(seed), seed(seed) {}

void column(Chunk& c, int x, int z, int& height, BlockType t, BlockType b) {
    auto h = std::max(height, 4);
    height = std::max(SEA_LEVEL, height);

    c.raise_height((float)std::max(h, SEA_LEVEL));
    for (int i = 0; i < h-4; i++)
        c.blocks[i][z][x] = {.type = BlockType::stone};
    for (int i = h-4; i < h-1; i++)
        c.blocks[i][z][x] = {.type = b};
    c.blocks[h-1][z][x] = {.type = t};
    for (int i = h; i < SEA_LEVEL; i++)
        c.blocks[i][z][x] = {.type = BlockType::water};
}

Chunk WorldGen::generate(World* world, ChunkPos pos) {
    Chunk chunk{pos};

    auto& high = chunk.high_map;
    auto b = gen_height_map(high, pos);

    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++) {
            auto& bm = biome_data[b[i][j]];
            column(chunk, i, j, high[i][j], bm.top_block, bm.bottom_block);
        }

    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++) {
            auto& bm = biome_data[b[i][j]];
            for (int k = 0; bm.decorations[k] && k < MAX_DECORATIONS; k++)
                bm.decorations[k]
                    ->apply(world, chunk, (int)seed, i, high[i][j], j);
        }

    chunk.is_dirty = true;

    return chunk;
}

mathpls::mat<int, 16, 16> WorldGen::gen_height_map(Heightmap& high, ChunkPos pos) {
    Basic bs[] = { 1, 2, 3, 4 };
    Octave os[] = {
        {5, 0}, {5, 1},
        {5, 2}, {5, 3},
        {5, 4}, {5, 5},
    };
    Combined cs[] = {
        {&bs[0], &bs[1]},
        {&bs[2], &bs[3]},
        {&os[3], &os[4]},
        {&os[1], &os[2]},
        {&os[1], &os[3]}
    };
    ExpScale
        n_h = {&os[0], 1.3f, 1.0f / 64.0f},
        n_m = {&cs[0], 1.0f, 1.0f / 256.0f},
        n_t = {&cs[1], 1.0f, 1.0f / 256.0f},
        n_r = {&cs[2], 1.0f, 1.0f / 8.0f},
        n_n = {&cs[3], 3.0f, 1.0f / 256.0f},
        n_p = {&cs[4], 3.0f, 1.0f / 256.0f};

    auto fh = mathpls::mat<float, 16, 16>::zero();
    auto b = mathpls::mat<int, 16, 16>::zero();

    using mathpls::clamp;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            auto x = (float)(pos.x + i);
            auto z = (float)(pos.z + j);

            float h = n_h.compute(noise, x, z),
                  m = n_m.compute(noise, x, z) * .5f + .5f,
                  t = n_t.compute(noise, x, z) * .5f + .5f,
                  r = n_r.compute(noise, x, z),
                  n = n_n.compute(noise, x, z),
                  p = n_p.compute(noise, x, z);

            n += (0 < p ? 1.f : -1.f) * std::pow(std::abs(p), (1.0f - n) * 3.0f);

            t -= 0.4f * n;
            t = clamp(t, 0.0f, 1.0f);

            auto biome_id = get_biome(h, m, t, n, n + h);
            auto& biome = biome_data[biome_id];

            n = std::pow(2.f, n) - 1;
            h = (0 < h ? 1.f : -1.f) * std::pow(std::abs(h), biome.exp);
            h = ((h * 8.0f) + (n * 96.0f)) * biome.scale + (biome.roughness * r * 2.0f);

            fh[i][j] = h + SEA_LEVEL;
            b[i][j] = biome_id;
        }
    }

    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++) {
            float v = 0;
            v += fh[clamp(i + 1, 0, 15)][clamp(j + 1, 0, 15)];
            v += fh[clamp(i + 1, 0, 15)][clamp(j - 1, 0, 15)];
            v += fh[clamp(i - 1, 0, 15)][clamp(j + 1, 0, 15)];
            v += fh[clamp(i - 1, 0, 15)][clamp(j - 1, 0, 15)];
            v *= .25f;
            high[i][j] = (int)std::round(v);
        }
    return b;
}
