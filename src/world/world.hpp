//
// Created by Ninter6 on 2024/7/19.
//

#pragma once

#include <string>
#include <optional>

#include "chunk.hpp"
#include "gen/worldgen.hpp"
#include "gen/worldsaver.hpp"

#include "math/ray.hpp"
#include "utils/tick.hpp"

#include "render/camera.hpp"
#include "render/render_type.hpp"

constexpr int SIGHT_DISTANCE = 2;
constexpr int WORLD_RADIUS = INT16_MAX;

struct PreloadInfo {
    int x, y, z; // 0 ~ 15
    BlockType type;
};

struct WorldInitInfo {
    std::string file = {};
    std::string name = "test";
    uint32_t seed;
};

class World {
public:
    World(const WorldInitInfo& initInfo);
    World(std::string_view filename);

    BlockType get_block(const mathpls::ivec3& pos) const;
    void set_block(const mathpls::ivec3& pos, BlockType type);
    void push_preload_block(ChunkPos pos, int x, int y, int z, BlockType);
    void check_preload_block(Chunk& c);

    std::optional<mathpls::ivec3> cast_block(Ray ray, bool previous = false) const;
    bool cast_and_set_block(const Ray& ray, BlockType type, bool previous = false);
    std::optional<mathpls::ivec3> camera_target_block(bool previous = false) const;
    bool set_camera_target_block(BlockType type, bool previous = false);

    void gen_world(ChunkPos min, ChunkPos max);
    void gen_camera_sight();
    void new_chunk(ChunkPos pos, Chunk&& chunk);
    void neighbor_chunk(Chunk& c, ChunkPos npos, int n);

    ChunkPos calcu_camera_chunk();
    std::pair<ChunkPos, ChunkPos> get_camera_sight() const;
    void calcu_new_chunk_camera_sight();

    void update();
    void save();

    DrawData get_draw_data(); // this will reset chunks' dirty flag
    std::unordered_map<ChunkPos, ChunkFace> get_dirty_chunk_data();

    std::optional<Chunk*> is_chunk_initialized(ChunkPos p) const; // if uninitialized, return nullopt

private:
    std::string name;
    uint32_t seed;

    std::unordered_map<ChunkPos, Chunk> map;
    std::unordered_multimap<ChunkPos, PreloadInfo> preload_blocks;

    Ticker ticker{50, 12000};

    std::unique_ptr<WorldGen> gen;
    WorldSaver saver;

    Camera* cam;
    ChunkPos camera_chunk{};
    std::pair<ChunkPos, ChunkPos> camera_sight, last_camera_sight;

    friend WorldSaver;
};
