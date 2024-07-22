//
// Created by Ninter6 on 2024/7/19.
//

#include <cassert>
#include "world.hpp"

World::World(const WorldInitInfo &initInfo) :
    generator{initInfo.seed},
    name{initInfo.name},
    cam{initInfo.camera}
{
    calcu_camera_chunk();
}

BlockType World::get_block(const mathpls::ivec3& pos) const {
    ChunkPos cp = {pos.x & ~15, pos.z & ~15};
    return map.at(cp).get_block(pos - mathpls::ivec3(cp));
}

void World::set_block(const mathpls::ivec3& pos, BlockType type) {
    ChunkPos cp = {pos.x & ~15, pos.z & ~15};
    assert(is_chunk_initialized(cp));
    map[cp].set_block(pos - mathpls::ivec3(cp), type);
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
            if (!is_chunk_initialized(p))
                map[p] = generator.generateChunk(p);
        }
}

void World::gen_camera_sight() {
    const auto& [min, max] = get_camera_sight();
    gen_world(min, max);
}

ChunkPos World::calcu_camera_chunk() {
    auto x = (int)cam->position.x;
    auto z = (int)cam->position.z;
    last_camera_chunk = camera_chunk;
    return (camera_chunk = {x & ~15, z & ~15});
}

std::pair<ChunkPos, ChunkPos> World::get_camera_sight() const {
    return {
        {camera_chunk.x - SIGHT_DISTANCE * 16, camera_chunk.z - SIGHT_DISTANCE * 16},
        {camera_chunk.x + SIGHT_DISTANCE * 16, camera_chunk.z + SIGHT_DISTANCE * 16}
    };
}

void World::calcu_new_chunk_camera_sight() {
    auto xmn = std::max(camera_chunk.x, last_camera_chunk.x) - SIGHT_DISTANCE * 16;
    auto zmn = std::max(camera_chunk.z, last_camera_chunk.z) - SIGHT_DISTANCE * 16;
    auto xmx = std::min(camera_chunk.x, last_camera_chunk.x) + SIGHT_DISTANCE * 16;
    auto zmx = std::min(camera_chunk.z, last_camera_chunk.z) + SIGHT_DISTANCE * 16;

    auto [min, max] = get_camera_sight();
    if (xmn == min.x && xmx == max.x && zmn == min.z && zmx == max.z) return;

    auto [xb, zb] = min;
    auto [xe, ze] = max;
    for (int x = xb; x <= xe; x += 16)
        for (int z = zb; z <= ze; z += 16)
            if (xmn > x || zmn > z || x > xmx || z > zmx)
                map[{x, z}].is_dirty = true;
}

void World::update() {
    calcu_camera_chunk();
    gen_camera_sight();
    calcu_new_chunk_camera_sight();
}

DrawData World::get_draw_data() {
    DrawData data;
    data.camera_visible_range = get_camera_sight();

    data.dirty_chunk = get_dirty_chunk_data();

    data.camera_target_block = camera_target_block(true);

    return data;
}

std::unordered_map<ChunkPos, std::vector<Face>> World::get_dirty_chunk_data() {
    auto [min, max] = get_camera_sight();
    auto [xb, zb] = min;
    auto [xe, ze] = max;

    std::unordered_map<ChunkPos, std::vector<Face>> dirty_chunks;
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

std::optional<Chunk*> World::is_chunk_initialized(ChunkPos p) {
    auto it = map.find(p);
    return it != map.end() ? std::optional<Chunk*>{&it->second} : std::nullopt;
}
