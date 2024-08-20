//
// Created by Ninter6 on 2024/7/19.
//

#include <cassert>
#include <utility>
#include <queue>

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
    assert(y >= 0);
    if (y < blocks.size()) return;
    blocks.resize((size_t)(std::ceil(y/63.99f)*64), Plane::zero());
}

Chunk::chunk_neighbor_r Chunk::find_chunk_neighbor(int y, int z, int x) {
    if (z < 0) {
        if (neighbor[2]) return neighbor[2]->find_chunk_neighbor(y, z + 16, x);
    } else if (z > 15) {
        if (neighbor[1]) return neighbor[1]->find_chunk_neighbor(y, z - 16, x);
    } else if (x < 0) {
        if (neighbor[3]) return neighbor[3]->find_chunk_neighbor(y, z, x + 16);
    } else if (x > 15) {
        if (neighbor[0]) return neighbor[0]->find_chunk_neighbor(y, z, x - 16);
    } else if (y >= 0 && y < blocks.size())
        return {this, y, z, x};
    return {};
}

BlockType Chunk::get_block(mathpls::ivec3 pos) const {
    assert(0 <= pos.x && pos.x < 16 && 0 <= pos.z && pos.z < 16);
    return block_type(pos.y, pos.z, pos.x);
}

void Chunk::set_block(mathpls::ivec3 pos, BlockType type) {
    assert(0 <= pos.x && pos.x < 16 && 0 <= pos.z && pos.z < 16);

    check_highmap(pos, type);

    auto& block = blocks[pos.y][pos.z][pos.x];
    if (block.type == type) return;

    const auto& o = ::get_block(block.type), n = ::get_block(type);
    block.type = type;

    check_neighbor(pos, o, n);
    check_brightness(pos, block.brightness, n->emission());

    is_dirty = true;
}


void Chunk::check_highmap(mathpls::ivec3 pos, BlockType type) {
    if (high_map[pos.x][pos.z] <= pos.y) {
        if (::get_block(type)->cast_light())
            high_map[pos.x][pos.z] = pos.y + 1;
        raise_height((float) pos.y);
    } else if (high_map[pos.x][pos.z] - 1 == pos.y && !::get_block(type)->cast_light())
        do { high_map[pos.x][pos.z]--;
        } while (!get_block(high_map[pos.x][pos.z] - 1, pos.z, pos.x)->cast_light());
}

void Chunk::check_neighbor(mathpls::ivec3 pos, BlockBase* o, BlockBase* n) {
    if ((o->transparent() && o->fluid()) || (n->transparent() && n->fluid())) {
        search_neighbors_plus(pos.y, pos.z, pos.x);
    } else {
        if ((o->fragmentary() || o->transparent()) != (n->fragmentary() || n->transparent()))
            new_neighbor(pos.y, pos.z, pos.x);
        if (n->renderable() && !o->renderable())
            blocks[pos.y][pos.z][pos.x].neighbors = search_neighbors(pos.y, pos.z, pos.x);
    }
}

void Chunk::check_brightness(mathpls::ivec3 pos, int16_t o, int16_t n) {
    if (o == n) return;
    if (o > n) unload_brightness(pos.y, pos.z, pos.x, o);
    load_brightness(pos.y, pos.z, pos.x, n);
}

void Chunk::load_brightness(int y, int z, int x, int16_t e) {
    if (e <= blocks[y][z][x].brightness) return;
    blocks[y][z][x].brightness = e;
    is_dirty = true;

    if (e-- < 2) return;
    if (y > 0 && !get_block(y-1, z, x)->cast_light())
        load_brightness(y-1, z, x, e);
    if (y < blocks.size()-1 && !get_block(y+1, z, x)->cast_light())
        load_brightness(y+1, z, x, e);
    if (z > 0 && !get_block(y, z-1, x)->cast_light())
        load_brightness(y, z-1, x, e);
    if (z < 15 && !get_block(y, z+1, x)->cast_light())
        load_brightness(y, z+1, x, e);
    if (x > 0 && !get_block(y, z, x-1)->cast_light())
        load_brightness(y, z, x-1, e);
    if (x < 15 && !get_block(y, z, x+1)->cast_light())
        load_brightness(y, z, x+1, e);
    if (z == 0 && neighbor[2] && !neighbor[2]->get_block(y, 15, x)->cast_light())
        neighbor[2]->load_brightness(y, 15, x, e);
    if (z == 15 && neighbor[1] && !neighbor[1]->get_block(y, 0, x)->cast_light())
        neighbor[1]->load_brightness(y, 0, x, e);
    if (x == 0 && neighbor[3] && !neighbor[3]->get_block(y, z, 15)->cast_light())
        neighbor[3]->load_brightness(y, z, 15, e);
    if (x == 15 && neighbor[0] && !neighbor[0]->get_block(y, z, 0)->cast_light())
        neighbor[0]->load_brightness(y, z, 0, e);
}

void Chunk::unload_brightness(int y, int z, int x, int16_t e) {
    if (e < 2) return;
    std::vector<chunk_neighbor_r> rl;

    std::queue<mathpls::ivec3> q;
    q.emplace(0);

    constexpr int md = 2*BlockBase::max_emission+1;
    bool p[md][md][md]{};

    while (!q.empty()) {
        auto dp = q.front(); q.pop();
        auto d = abs(dp.x) + abs(dp.y) + abs(dp.z);
        auto ny = y + dp.y, nz = z + dp.z, nx = x + dp.x;

        auto& f = p[md/2+dp.y][md/2+dp.z][md/2+dp.x];
        if (f) continue;
        f = true;

        auto r = find_chunk_neighbor(ny,nz, nx);
        if (!r.c || r.c->get_block(r.y, r.z, r.x)->cast_light()) continue;
        if (e - d >= r.c->blocks[r.y][r.z][r.x].brightness) {
            r.c->blocks[r.y][r.z][r.x].brightness = 0;
            r.c->is_dirty = true;
        } else rl.push_back(r);

        if (e - d < 1) continue;
        q.emplace(dp.x, dp.y + 1, dp.z);
        q.emplace(dp.x, dp.y - 1, dp.z);
        q.emplace(dp.x, dp.y, dp.z + 1);
        q.emplace(dp.x, dp.y, dp.z - 1);
        q.emplace(dp.x + 1, dp.y, dp.z);
        q.emplace(dp.x - 1, dp.y, dp.z);
    }

    for (auto&& [c, y, z, x] : rl) {
        c->load_brightness(y, z, x, std::exchange(c->blocks[y][z][x].brightness, 0));
    }
}

BlockBase* Chunk::get_block(int y, int z, int x) const {
    return ::get_block(block_type(y, z, x));
}

BlockType Chunk::block_type(int y, int z, int x) const {
    if (y >= blocks.size() || y < 0) return BlockType::air;
    return blocks[y][z][x].type;
}

FaceMask Chunk::block_neighbors(int y, int z, int x) const {
    if (y >= blocks.size() || y < 0) return 0b111111;
    return blocks[y][z][x].neighbors;
}

int16_t Chunk::block_brightness(int y, int z, int x) const {
    if (y >= blocks.size() || y < 0) return 0;
    return blocks[y][z][x].brightness;
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

void Chunk::update_height_map() {
    for (int z = 0; z < 16; z++)
        for (int x = 0; x < 16; x++)
            for (int y = (int)blocks.size(); y > 0; y--)
                if (get_block(y-1, z, x)->cast_light()) {
                    high_map[x][z] = y;
                    break;
                }
}

bool is_visible_neighbor(BlockType a, BlockType n) {
    auto&& nb = ::get_block(n);
    return nb->fragmentary() ||
          (nb->transparent() && (!nb->fluid() || a != n));
}

FaceMask Chunk::search_neighbors(int y, int z, int x) const {
#define BT(y_, z_, x_) (blocks[y_][z_][x_].type)
#define N_BT(n_, y_, z_, x_) (neighbor[n_]->block_type(y_, z_, x_))
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
    return y < h ? .1f : 1.f;
}

float Chunk::brightness(int facing, int y, int z, int x) const {
    auto b = [&]() -> int {
        switch (facing) {
            case 0:
                return (z == 0 ? (neighbor[2] ? neighbor[2]->block_brightness(y, 15, x) : 0) : blocks[y][z-1][x].brightness);
            case 1:
                return (x == 15 ? (neighbor[0] ? neighbor[0]->block_brightness(y, z, 0) : 0) : blocks[y][z][x+1].brightness);
            case 2:
                return (z == 15 ? (neighbor[1] ? neighbor[1]->block_brightness(y, 0, x) : 0) : blocks[y][z+1][x].brightness);
            case 3:
                return (x == 0 ? (neighbor[3] ? neighbor[3]->block_brightness(y, z, 15) : 0) : blocks[y][z][x-1].brightness);
            case 4:
                return y < blocks.size() - 1 ? blocks[y+1][z][x].brightness : 0;
            case 5:
                return y > 0 ? blocks[y-1][z][x].brightness : 0;
            default:
                return 0;
        }
    }();
    if (auto e  = get_block(y, z, x)->emission(); e > 0)
        return (float)(e > b ? e : b) / BlockBase::max_emission;
    return (float)b / BlockBase::max_emission;
}

void Chunk::get_block_face(int y, int z, int x, FaceMask mask, ChunkFace& cf) const {
    auto&& b = get_block(y, z, x);
    auto pos = mathpls::vec3(position) + mathpls::vec3(x, y, z);
    int i = 0;
    if (b->special()) {
        auto bf = b->get_special_faces(mask);
        while (bf[i]) {
            bf[i]->pos = pos;
            bf[i]->sunIntensity = (uint16_t)(calcu_sun_intensity(bf[i]->facing, y, z, x) * UINT16_MAX);
            bf[i]->lightIntensity = (uint16_t)(brightness(bf[i]->facing, y, z, x) * UINT16_MAX);
            cf.special_faces.push_back(*bf[i++]);
        }
    } else {
        auto bf = b->get_faces(mask);
        while (bf[i]) {
            bf[i]->pos = pos;
            bf[i]->sunIntensity = (uint16_t)(calcu_sun_intensity(bf[i]->facing, y, z, x) * UINT16_MAX);
            bf[i]->lightIntensity = (uint16_t)(brightness(bf[i]->facing, y, z, x) * UINT16_MAX);
            cf.normal_faces.push_back(*bf[i++]);
        }
    }
}
