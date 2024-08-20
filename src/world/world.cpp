//
// Created by Ninter6 on 2024/7/19.
//

#include <cassert>
#include "world.hpp"

World::World(const WorldInitInfo &initInfo) :
    saver{initInfo.file.empty() ?
        worldname2filename(initInfo.name) : initInfo.file},
    name{initInfo.name},
    seed{initInfo.seed},
    cam{initInfo.camera}
{
    if (!initInfo.file.empty())
        saver.load(*this);
    gen = std::make_unique<WorldGen>(seed);
    calcu_camera_chunk();
}

BlockType World::get_block(const mathpls::ivec3& pos) const {
    ChunkPos cp = {pos.x & ~15, pos.z & ~15};
    assert(is_chunk_initialized(cp));
    return map.at(cp).get_block(pos - mathpls::ivec3(cp));
}

void World::set_block(const mathpls::ivec3& pos, BlockType type) {
    ChunkPos cp = {pos.x & ~15, pos.z & ~15};
    if (auto p = is_chunk_initialized(cp))
        (*p)->set_block(pos - mathpls::ivec3(cp), type);
    else
        push_preload_block(cp, pos.x & 15, pos.y, pos.z & 15, type);
}

void World::push_preload_block(ChunkPos pos, int x, int y, int z, BlockType type) {
    preload_blocks.emplace(pos, PreloadInfo{x, y, z, type});
}

void World::check_preload_block(Chunk& c) {
    if (preload_blocks.empty()) return;
    auto [b, e] = preload_blocks.equal_range(c.position);
    for (auto it = b; it != e; ++it)
        c.set_block({it->second.x, it->second.y, it->second.z}, it->second.type);
    preload_blocks.erase(b, e);
}

std::optional<mathpls::ivec3> World::cast_block(Ray ray, bool previous) const {
    mathpls::ivec3 pp = ray.origin;
    if (::get_block(get_block(pp))->collidable())
        return pp;

    constexpr int max_step = 8;
    for (int i = 0; i < max_step; ++i)
        if (auto pos = ray.step(); ::get_block(get_block(pos))->collidable())
            return previous ? pp : mathpls::ivec3{pos};
        else pp = pos;
    return std::nullopt;
}

bool World::cast_and_set_block(const Ray& ray, BlockType type, bool previous) {
    if (auto pos = cast_block(ray, previous)) {
        set_block(*pos, type);
        return true;
    }
    return false;
}

std::optional<mathpls::ivec3> World::camera_target_block(bool previous) const {
    return cast_block({cam->position, cam->forward}, previous);
}

bool World::set_camera_target_block(BlockType type, bool previous) {
    return cast_and_set_block({cam->position, cam->forward}, type, previous);
}

void World::gen_world(ChunkPos min, ChunkPos max) {
    auto [xb, zb] = min;
    auto [xe, ze] = max;
    for (int x = xb; x <= xe; x += 16)
        for (int z = zb; z <= ze; z += 16) {
            ChunkPos p = {x, z};
            if (!is_chunk_initialized(p)) {
                new_chunk(p, gen->generate(this, p));
            }
        }
}

void World::new_chunk(ChunkPos pos, Chunk&& chunk) {
    auto [it, _] = map.emplace(pos, std::move(chunk));
    check_preload_block(it->second);
    auto [x, z] = pos;
    neighbor_chunk(it->second, {x + 16, z}, 0);
    neighbor_chunk(it->second, {x, z + 16}, 1);
    neighbor_chunk(it->second, {x, z - 16}, 2);
    neighbor_chunk(it->second, {x - 16, z}, 3);
    it->second.update_neighbors();
}

void World::neighbor_chunk(Chunk& c, ChunkPos npos, int n) { // NOLINT(*-make-member-function-const)
    if (auto p = is_chunk_initialized(npos)) {
        c.neighbor[n] = *p;
        (*p)->neighbor[3-n] = &c;
        (*p)->update_neighbors();
        (*p)->is_dirty = true;
    }
}

void World::gen_camera_sight() {
    const auto& [min, max] = camera_sight;
    gen_world(min, max);
}

ChunkPos World::calcu_camera_chunk() {
    last_camera_sight = camera_sight;
    if (cam->is_dirty) {
        auto x = (int) cam->position.x;
        auto z = (int) cam->position.z;
        camera_chunk = {x & ~15, z & ~15};
        camera_sight = get_camera_sight();
    }
    return camera_chunk;
}

std::pair<ChunkPos, ChunkPos> World::get_camera_sight() const {
    auto d = cam->forward;
    auto r = mathpls::cross({0, 1, 0}, d);
    auto u = mathpls::cross(d, r);
    mathpls::mat3 m = {r, u, d};

    auto w = SIGHT_DISTANCE * 16 * std::tan(cam->frustum.fovy);
    auto h = w / cam->frustum.asp;

    mathpls::vec3 v[4] = {
        { w, h, cam->frustum.far},
        {-w, h, cam->frustum.far},
        { w,-h, cam->frustum.far},
        {-w,-h, cam->frustum.far}
    };

    int mnx = 0, mnz = 0, mxx = 0, mxz = 0;
    for (auto&& i : v) {
        auto p = m * i;
        int x = floor(p.x);
        int z = floor(p.z);

        if (x < mnx) mnx = x;
        if (x > mxx) mxx = x;
        if (z < mnz) mnz = z;
        if (z > mxz) mxz = z;
    }

    mnx = std::clamp(mnx, -SIGHT_DISTANCE * 16, SIGHT_DISTANCE * 16);
    mnz = std::clamp(mnz, -SIGHT_DISTANCE * 16, SIGHT_DISTANCE * 16);
    mxx = std::clamp(mxx, -SIGHT_DISTANCE * 16, SIGHT_DISTANCE * 16);
    mxz = std::clamp(mxz, -SIGHT_DISTANCE * 16, SIGHT_DISTANCE * 16);

    return {
        {camera_chunk.x + (mnx & ~15), camera_chunk.z + (mnz & ~15)},
        {camera_chunk.x + (mxx & ~15), camera_chunk.z + (mxz & ~15)}
    };
}

void World::calcu_new_chunk_camera_sight() {
    if (!cam->is_dirty ||
        (camera_sight.first == last_camera_sight.first &&
         camera_sight.second == last_camera_sight.second)) return;

    auto xmn = std::max(camera_sight.first.x, last_camera_sight.first.x);
    auto zmn = std::max(camera_sight.first.z, last_camera_sight.first.z);
    auto xmx = std::min(camera_sight.second.x, last_camera_sight.second.x);
    auto zmx = std::min(camera_sight.second.z, last_camera_sight.second.z);

    auto [xb, zb] = camera_sight.first;
    auto [xe, ze] = camera_sight.second;
    for (int x = xb; x <= xe; x += 16)
        for (int z = zb; z <= ze; z += 16)
            if (xmn > x || zmn > z || x > xmx || z > zmx)
                map[{x, z}].is_dirty = true;
}

void World::update() {
    ticker.tick();
    calcu_camera_chunk();
    gen_camera_sight();
    calcu_new_chunk_camera_sight();
}

void calcu_sun(const Ticker& tk, mathpls::vec3& dir, float& I) {
    float t = (float) tk.now / (float) tk.tick_per_day * 2;
    if (t > 1) {
        I = 52428.8f * std::pow(t - 1.5f, 18.f);
        t -= 1;
    } else
        I = -204.8f * std::pow(t - .5f, 8.f) + 1.f;
    I = I * .9f + .05f;
    dir = {cos(t * 3.1416f), sin(t * 3.1416f), cos(3.14f*tk.day/180)*.3f};
    dir.normalize();
}

DrawData World::get_draw_data() {
    DrawData data;
    data.camera_visible_range = camera_sight;

    data.dirty_chunk = get_dirty_chunk_data();

    data.camera_target_block = camera_target_block(true);

    calcu_sun(ticker, data.sun_dir, data.sun_I);

    return data;
}

std::unordered_map<ChunkPos, ChunkFace> World::get_dirty_chunk_data() {
    auto [min, max] = camera_sight;
    auto [xb, zb] = min;
    auto [xe, ze] = max;

    std::unordered_map<ChunkPos, ChunkFace> dirty_chunks;
    for (int x = xb; x <= xe; x += 16)
        for (int z = zb; z <= ze; z += 16) {
            ChunkPos p = {x, z};
            if (map[p].is_dirty) {
                map[p].is_dirty = false;
                dirty_chunks.emplace(map[p].position, map[p].get_render_faces());
            }
        }
    return dirty_chunks;
}

std::optional<Chunk*> World::is_chunk_initialized(ChunkPos p) const {
    auto it = map.find(p);
    return it != map.end() ? std::optional<Chunk*>{const_cast<Chunk*>(&it->second)} : std::nullopt;
}

void World::save() {
    puts("\n正在保存世界...");
    saver.save(*this);
    puts("保存完毕");
}