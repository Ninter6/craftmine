//
// Created by Ninter6 on 2024/7/18.
//

#pragma once

#include <vector>
#include <optional>
#include <unordered_map>

#include "math/mathpls.h"

struct CameraUBO {
    mathpls::mat4 proj;
    mathpls::mat4 view;
};

struct color_u32 : mathpls::vec<uint8_t, 4> {
    using vec::vec;
    operator uint32_t() const {return std::bit_cast<uint32_t>(*this);} // NOLINT(*-explicit-constructor)
};

struct Face {
    mathpls::vec3 pos{};
    int facing{};
    float posOffset{};
    float texIndex{};
    float lightIntensity = 1.f;
    uint32_t color = color_u32{255};
};

struct ChunkPos {
    int32_t x, z;
    bool operator==(const ChunkPos& o) const {return x == o.x && z == o.z;}
    operator mathpls::vec3() const {return mathpls::vec3(x, 0, z);}
    operator mathpls::ivec3() const {return mathpls::ivec3(x, 0, z);}
};

template <>
struct std::hash<ChunkPos> {
    std::size_t operator()(const ChunkPos& pos) const noexcept {
        return ((size_t)pos.x << 32) | pos.z;
    }
};

struct DrawData {
    std::unordered_map<ChunkPos, std::vector<Face>> dirty_chunk;
    std::pair<ChunkPos, ChunkPos> camera_visible_range;
    std::optional<mathpls::ivec3> camera_target_block;
};
