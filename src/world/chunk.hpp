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
    int16_t brightness = 0;
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

    struct chunk_neighbor_r {
        Chunk* c = nullptr;
        int y, z, x;
    };
    chunk_neighbor_r find_chunk_neighbor(int x, int y, int z); // accept pos not within [0, 15]

    BlockType get_block(mathpls::ivec3 pos) const;
    void set_block(mathpls::ivec3 pos, BlockType type);

    void check_highmap(mathpls::ivec3 pos, BlockType type);
    void check_neighbor(mathpls::ivec3 pos, BlockBase* o, BlockBase* n);
    void check_brightness(mathpls::ivec3 pos, int16_t o, int16_t n);

    void load_brightness(int y, int z, int x, int16_t e);
    void unload_brightness(int y, int z, int x, int16_t e);

    BlockBase* get_block(int y, int z, int x) const;
    BlockType block_type(int y, int z, int x) const;
    FaceMask block_neighbors(int y, int z, int x) const;
    int16_t block_brightness(int y, int z, int x) const;

    bool is_renderable(int y, int z, int x) const;
    bool is_fragmentary(int y, int z, int x) const;
    bool is_transparent(int y, int z, int x) const;

    void update_height_map();

    FaceMask search_neighbors(int y, int z, int x) const;
    void search_neighbors_plus(int y, int z, int x);
    void update_neighbors();
    void new_neighbor(int y, int z, int x); // reverse the neighbor value of blocks around the center block

    float calcu_sun_intensity(int facing, int y, int z, int x) const;
    float brightness(int facing, int y, int z, int x) const;

    void get_block_face(int y, int z, int x, FaceMask mask, ChunkFace& cf) const;

    ChunkFace get_render_faces() const;

};

// update
// update neighbors
// get render faces