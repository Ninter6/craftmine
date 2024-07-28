//
// Created by Ninter6 on 2024/7/19.
//

#include <cassert>

#include "chunk.hpp"

Chunk::Chunk(ChunkPos pos) : position(pos), blocks(64, Plane::zero()) {}

std::vector<Face> Chunk::get_render_faces() const {
    std::vector<Face> rst;

    for (int y = 0; y < blocks.size(); y++)
        for (int z = 0; z < 16; z++)
            for (int x = 0; x < 16; x++) {
                if (!is_renderable(y, z, x)) continue;
                auto mask = blocks[y][z][x].neighbors;
                get_block_face(y, z, x, mask, rst);
            }

    return rst;
}

void Chunk::raise_height(float y) {
    assert(y > 0);
    if (y < blocks.size()) return;
    blocks.resize((size_t)(std::ceil(y/(63.99))*64), Plane::zero());
}

BlockType Chunk::get_block(mathpls::ivec3 pos) const {
    assert(0 <= pos.x && pos.x < 16 && 0 <= pos.z && pos.z < 16);
    if (0 < pos.y && pos.y >= blocks.size()) return BlockType::air;
    return blocks[pos.y][pos.z][pos.x].type;
}

void Chunk::set_block(mathpls::ivec3 pos, BlockType type) {
    assert(0 <= pos.x && pos.x < 16 && 0 <= pos.z && pos.z < 16);
    if (high_map[pos.x][pos.z] <= pos.y) {
        high_map[pos.x][pos.z] = pos.y + 1;
        raise_height((float) pos.y);
    }
    auto& block = blocks[pos.y][pos.z][pos.x];
    if (::get_block(type)->fragmentary() != ::get_block(block.type)->fragmentary())
        new_neighbor(pos.y, pos.z, pos.x);
    if (::get_block(type)->renderable() && !::get_block(block.type)->renderable())
        block.neighbors = search_neighbors(pos.y, pos.z, pos.x);
    block.type = type;
    is_dirty = true;
}

BlockBase* Chunk::get_block(int y, int z, int x) const {
    if (y >= blocks.size()) return ::get_block(BlockType::air);
    return ::get_block(blocks[y][z][x].type);
}

bool Chunk::is_renderable(int y, int z, int x) const {
    return get_block(y, z, x)->renderable();
}

bool Chunk::is_transparent(int y, int z, int x) const {
    return get_block(y, z, x)->fragmentary();
}

FaceMask Chunk::search_neighbors(int y, int z, int x) const {
    if (y >= high_map[x][z]) return 0;
    FaceMask rst = 0;
    if (z == 0 ? !neighbor[2] || neighbor[2]->is_transparent(y, 15, x) : is_transparent(y, z - 1, x)) rst |= (int)FaceType::front;
    if (z == 15 ? !neighbor[1] || neighbor[1]->is_transparent(y, 0, x) : is_transparent(y, z + 1, x)) rst |= (int)FaceType::back;
    if (x == 15 ? !neighbor[0] || neighbor[0]->is_transparent(y, z, 0) : is_transparent(y, z, x + 1)) rst |= (int)FaceType::left;
    if (x == 0 ? !neighbor[3] || neighbor[3]->is_transparent(y, z, 15) : is_transparent(y, z, x - 1)) rst |= (int)FaceType::right;
    if (y == high_map[x][z]-1 || is_transparent(y + 1, z, x)) rst |= (int)FaceType::top;
    if (y == 0 || is_transparent(y - 1, z, x)) rst |= (int)FaceType::bottom;
    return rst;
}

void Chunk::update_neighbors() {
    for (int y = 0; y < blocks.size(); y++)
        for (int z = 0; z < 16; z++)
            for (int x = 0; x < 16; x++)
                if (is_renderable(y, z, x))
                    blocks[y][z][x].neighbors = search_neighbors(y, z, x);
}

void Chunk::new_neighbor(int y, int z, int x) {
    if (y < high_map[x][z] - 1 && is_renderable(y+1, z, x))
        blocks[y+1][z][x].neighbors ^= (int)FaceType::bottom;
    if (y > 0 && is_renderable(y-1, z, x))
        blocks[y-1][z][x].neighbors ^= (int)FaceType::top;
    if (z < 15 && is_renderable(y, z+1, x))
        blocks[y][z+1][x].neighbors ^= (int)FaceType::front;
    if (z > 0 && is_renderable(y, z-1, x))
        blocks[y][z-1][x].neighbors ^= (int)FaceType::back;
    if (x < 15 && is_renderable(y, z, x+1))
        blocks[y][z][x+1].neighbors ^= (int)FaceType::right;
    if (x > 0 && is_renderable(y, z, x-1))
        blocks[y][z][x-1].neighbors ^= (int)FaceType::left;
    if (z == 15 && neighbor[1] && neighbor[1]->is_renderable(y, 0, x)) {
        neighbor[1]->blocks[y][0][x].neighbors ^= (int) FaceType::front;
        neighbor[1]->is_dirty = true;
    } else if (z == 0 && neighbor[2] && neighbor[2]->is_renderable(y, 15, x)) {
        neighbor[2]->blocks[y][15][x].neighbors ^= (int) FaceType::back;
        neighbor[2]->is_dirty = true;
    } if (x == 15 && neighbor[0] && neighbor[0]->is_renderable(y, z, 0)) {
        neighbor[0]->blocks[y][z][0].neighbors ^= (int) FaceType::right;
        neighbor[0]->is_dirty = true;
    } else if (x == 0 && neighbor[3] && neighbor[3]->is_renderable(y, z, 15)) {
        neighbor[3]->blocks[y][z][15].neighbors ^= (int) FaceType::left;
        neighbor[3]->is_dirty = true;
    }
}

void Chunk::get_block_face(int y, int z, int x, FaceMask mask, std::vector<Face>& faces) const {
    auto bf = get_block(y, z, x)->get_faces(mask);
    int i = 0;
    while (bf[i]) {
        bf[i]->pos = mathpls::vec3(position) + mathpls::vec3(x, y, z);
        faces.push_back(*bf[i++]);
    }
}
