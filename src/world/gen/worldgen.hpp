//
// Created by Ninter6 on 2024/7/26.
//

#pragma once

#include "noise_factory.hpp"

#include "world/chunk.hpp"

class World;

struct WorldGen {
public:
    WorldGen(uint32_t seed);

    Chunk generate(World* world, ChunkPos pos);

    mathpls::mat<int, 16, 16> gen_height_map(Heightmap& high, ChunkPos pos);

private:
    uint32_t seed;
    PNoise noise;
};