//
// Created by Ninter6 on 2024/7/21.
//

#pragma once

#include "chunk.hpp"
#include "math/noise.hpp"

struct WorldGenerator {
    size_t seed;

    Chunk generateChunk(ChunkPos pos) const;
};
