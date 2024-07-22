//
// Created by Ninter6 on 2024/7/19.
//

#pragma once

#include <bit>
#include <array>
#include <optional>

#include "math/mathpls.h"
#include "render/render_type.hpp"

enum class BlockType : uint8_t {
    air = 0,
    grass,
    dirt,
    stone,
    water,
    wooden_plank,
    leaves,

    NUM_BLOCKS
};

enum class FaceType : uint8_t {
    front = 1,
    left = 2, // (x+)
    back = 4,
    right = 8, // (x-)
    top = 16,
    bottom = 32,
};
using FaceMask = uint8_t;

struct BlockBase {
    [[nodiscard]] virtual bool renderable() const { return true; }
    [[nodiscard]] virtual bool transparent() const {return false;} // if one isn't renderable, this should be true
    [[nodiscard]] virtual bool collidable() const {return true;}

    [[nodiscard]] virtual std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const = 0;
};

BlockBase* get_block(BlockType type);

template <class T>
struct Block : BlockBase {
    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override {
        auto& faces = static_cast<const T*>(this)->faces;

        std::array<std::optional<Face>, 6> res;
        for (int i = 0, j = 0; mask; mask >>= 1, j++)
            if (mask&1) res[i++] = faces[j];

        return res;
    }
};

struct Air : BlockBase {
    [[nodiscard]] bool renderable() const override { return false; }
    [[nodiscard]] bool transparent() const override {return true;}
    [[nodiscard]] bool collidable() const override {return false;}
    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override {return {};}
};

struct Grass : Block<Grass> {
    const Face faces[6] = {
        {0, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 2, 0, 1},
        {0, 3, 0, 1},
        {0, 4, 0, 0},
        {0, 5, 0, 2}
    };
};

struct Dirt : Block<Dirt> {
    const Face faces[6] = {
        {0, 0, 0, 2},
        {0, 1, 0, 2},
        {0, 2, 0, 2},
        {0, 3, 0, 2},
        {0, 4, 0, 2},
        {0, 5, 0, 2}
    };
};

struct Stone : Block<Stone> {
    const Face faces[6] = {
        {0, 0, 0, 3},
        {0, 1, 0, 3},
        {0, 2, 0, 3},
        {0, 3, 0, 3},
        {0, 4, 0, 3},
        {0, 5, 0, 3}
    };
};
