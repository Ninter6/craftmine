//
// Created by Ninter6 on 2024/7/19.
//

#pragma once

#include "block/block.hpp"

#include <vector>

#define CHUNK_SIZE (ivec3(16, 0, 16))

struct BlockData {
    BlockType type = BlockType::air;
    FaceMask neighbors = 0b111111;
};

using Plane = mathpls::mat<BlockData, 16, 16>;
using Heightmap = mathpls::mat<int, 16, 16>;

struct Chunk {
    Chunk(ChunkPos position = {});

    ChunkPos position;

    // 3D array, y - z - x
    std::vector<Plane> blocks;
    Chunk* neighbor[4]{}; // chunk neighbors

    Heightmap high_map{};
    int max_height{};

    bool is_dirty = false;

    void raise_height(float y); // raise to the smallest sum of 64 that large than y

    BlockType get_block(mathpls::ivec3 pos) const;
    void set_block(mathpls::ivec3 pos, BlockType type);

    BlockBase* get_block(int y, int z, int x) const;

    bool is_renderable(int y, int z, int x) const;
    bool is_transparent(int y, int z, int x) const;

    void update_height_map();

    FaceMask search_neighbors(int y, int z, int x) const;
    void update_neighbors();
    void new_neighbor(int y, int z, int x); // reverse the neighbor value of blocks around the center block

    void get_block_face(int y, int z, int x, FaceMask mask, std::vector<Face>& faces) const;

    std::vector<Face> get_render_faces() const;

};

// update
// update neighbors
// get render faces