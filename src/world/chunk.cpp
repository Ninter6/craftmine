//
// Created by Ninter6 on 2024/7/19.
//

#include <cassert>

#include "chunk.hpp"

Chunk::Chunk(ChunkPos pos) : position(pos), blocks(64, Plane::zero()) {}

ChunkFace Chunk::get_render_faces() const {
    ChunkFace rst;

    for (int y = 0; y < blocks.size(); y++)
        for (int z = 0; z < 16; z++)
            for (int x = 0; x < 16; x++) {
                if (!is_renderable(y, z, x)) continue;
                if (auto mask = blocks[y][z][x].neighbors)
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
        if (::get_block(type)->cast_sunlight())
            high_map[pos.x][pos.z] = pos.y + 1;
        raise_height((float) pos.y);
    } else if (high_map[pos.x][pos.z] - 1 == pos.y && !::get_block(type)->cast_sunlight())
        do { high_map[pos.x][pos.z]--;
        } while (!get_block(high_map[pos.x][pos.z] - 1, pos.z, pos.x)->cast_sunlight());
    auto& block = blocks[pos.y][pos.z][pos.x];
    if (block.type == type) return;

    auto&& o = ::get_block(block.type), n = ::get_block(type);
    block.type = type;
    if ((o->transparent() && o->fluid()) || (n->transparent() && n->fluid())) {
        search_neighbors_plus(pos.y, pos.z, pos.x);
    } else {
        if ((o->fragmentary() || o->transparent()) != (n->fragmentary() || n->transparent()))
            new_neighbor(pos.y, pos.z, pos.x);
        if (n->renderable() && !o->renderable())
            block.neighbors = search_neighbors(pos.y, pos.z, pos.x);
    }

    is_dirty = true;
}

BlockBase* Chunk::get_block(int y, int z, int x) const {
    if (y >= blocks.size()) return ::get_block(BlockType::air);
    return ::get_block(blocks[y][z][x].type);
}

bool Chunk::is_renderable(int y, int z, int x) const {
    return get_block(y, z, x)->renderable();
}

bool Chunk::is_fragmentary(int y, int z, int x) const {
    return get_block(y, z, x)->fragmentary();
}

bool Chunk::is_transparent(int y, int z, int x) const {
    return get_block(y, z, x)->transparent();
}

bool is_visible_neighbor(BlockType a, BlockType n) {
    auto&& nb = ::get_block(n);
    return nb->fragmentary() ||
          (nb->transparent() && (!nb->fluid() || a != n));
}

FaceMask Chunk::search_neighbors(int y, int z, int x) const {
#define BT(y_, z_, x_) (blocks[y_][z_][x_].type)
#define N_BT(n_, y_, z_, x_) (neighbor[n_]->blocks[y_][z_][x_].type)
    if (y >= high_map[x][z]) return 0b111111;
    FaceMask rst = 0;
    auto a = BT(y, z, x);
    if (z == 0 ? !neighbor[2] || is_visible_neighbor(a, N_BT(2, y, 15, x)) : is_visible_neighbor(a, BT(y, z - 1, x))) rst |= (int)FaceType::front;
    if (z == 15 ? !neighbor[1] || is_visible_neighbor(a, N_BT(1, y, 0, x)) : is_visible_neighbor(a, BT(y, z + 1, x))) rst |= (int)FaceType::back;
    if (x == 15 ? !neighbor[0] || is_visible_neighbor(a, N_BT(0, y, z, 0)) : is_visible_neighbor(a, BT(y, z, x + 1))) rst |= (int)FaceType::left;
    if (x == 0 ? !neighbor[3] || is_visible_neighbor(a, N_BT(3, y, z, 15)) : is_visible_neighbor(a, BT(y, z, x - 1))) rst |= (int)FaceType::right;
    if (y == high_map[x][z]-1 || is_visible_neighbor(a, BT(y + 1, z, x))) rst |= (int)FaceType::top;
    if (y == 0 || is_visible_neighbor(a, BT(y - 1, z, x))) rst |= (int)FaceType::bottom;
    return rst;
}

void Chunk::search_neighbors_plus(int y, int z, int x) {
    blocks[y][z][x].neighbors = search_neighbors(y, z, x);
    if (y < high_map[x][z] - 1 && is_renderable(y+1, z, x))
        blocks[y+1][z][x].neighbors = search_neighbors(y + 1, z, x);
    if (y > 0 && is_renderable(y-1, z, x))
        blocks[y-1][z][x].neighbors = search_neighbors(y - 1, z, x);
    if (z < 15 && is_renderable(y, z+1, x))
        blocks[y][z+1][x].neighbors = search_neighbors(y, z + 1, x);
    else if (neighbor[1] && neighbor[1]->is_renderable(y, 0, x))
        neighbor[1]->blocks[y][0][x].neighbors = neighbor[1]->search_neighbors(y, 0, x);
    if (z > 0 && is_renderable(y, z-1, x))
        blocks[y][z-1][x].neighbors = search_neighbors(y, z - 1, x);
    else if (neighbor[2] && neighbor[2]->is_renderable(y, 15, x))
        neighbor[2]->blocks[y][15][x].neighbors = neighbor[2]->search_neighbors(y, 15, x);
    if (x < 15 && is_renderable(y, z, x+1))
        blocks[y][z][x+1].neighbors = search_neighbors(y, z, x + 1);
    else if (neighbor[0] && neighbor[0]->is_renderable(y, z, 0))
        neighbor[0]->blocks[y][z][0].neighbors = neighbor[0]->search_neighbors(y, z, 0);
    if (x > 0 && is_renderable(y, z, x-1))
        blocks[y][z][x-1].neighbors = search_neighbors(y, z, x - 1);
    else if (neighbor[3] && neighbor[3]->is_renderable(y, z, 15))
        neighbor[3]->blocks[y][z][15].neighbors = neighbor[3]->search_neighbors(y, z, 15);
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

float Chunk::calcu_sun_intensity(int facing, int y, int z, int x) const {
    int h = 1919810;
    switch (facing) {
        case 0:
            h = z == 0 ? (neighbor[2] ? neighbor[2]->high_map[x][15] : 0) : high_map[x][z-1];
            break;
        case 1:
            h = x == 15 ? (neighbor[0] ? neighbor[0]->high_map[0][z] : 0) : high_map[x+1][z];
            break;
        case 2:
            h = z == 15 ? (neighbor[1] ? neighbor[1]->high_map[x][0] : 0) : high_map[x][z+1];
            break;
        case 3:
            h = x == 0 ? (neighbor[3] ? neighbor[3]->high_map[15][z] : 0) : high_map[x-1][z];
            break;
        case 4:
            h = high_map[x][z] - 1;
        default:
            break;
    }
    return y < h ? .314f : 1.f;
}

void Chunk::get_block_face(int y, int z, int x, FaceMask mask, ChunkFace& cf) const {
    auto&& b = get_block(y, z, x);
    auto pos = mathpls::vec3(position) + mathpls::vec3(x, y, z);
    int i = 0;
    if (b->special()) {
        auto bf = b->get_special_faces(mask);
        while (bf[i]) {
            bf[i]->pos = pos;
            bf[i]->sunIntensity = calcu_sun_intensity(bf[i]->facing, y, z, x);
            cf.special_faces.push_back(*bf[i++]);
        }
    } else {
        auto bf = b->get_faces(mask);
        while (bf[i]) {
            bf[i]->pos = pos;
            bf[i]->sunIntensity = calcu_sun_intensity(bf[i]->facing, y, z, x);
            cf.normal_faces.push_back(*bf[i++]);
        }
    }
}
