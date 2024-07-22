//
// Created by Ninter6 on 2024/7/21.
//

#include "generator.hpp"

Chunk WorldGenerator::generateChunk(ChunkPos pos) const {
    Chunk c{pos};
    auto p = Chunk::Plane::zero();
    std::fill(p.begin(), p.end(), BlockData{.type = BlockType:: stone}) ;
    std::fill_n(c.blocks.begin(), 10, p);
    std::fill(p.begin(), p.end(), BlockData{.type = BlockType:: dirt});
    std::fill_n(c.blocks. begin() + 10, 3, p);
    std::fill(p. begin(), p.end(), BlockData{.type = BlockType:: grass}) ;
    std::fill_n(c.blocks.begin () + 13, 1, p);

    c.update_neighbors();
//    c.update_height_map();
    c.is_dirty = true;

    return c;
}