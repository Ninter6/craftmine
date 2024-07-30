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
    grass_block,
    dirt,
    stone,
    sand,
    snow,
    podzol,
    log,
    leaf,
    pine_log,
    pine_leaf,
    grass,
    rose,
    dandelion,
    shrub,
    water,
    wooden_plank,

    MAX_BLOCKS
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

namespace block {

struct BlockBase {
    [[nodiscard]] virtual bool renderable() const { return true; }

    [[nodiscard]] virtual bool fragmentary() const { return false; } // if one isn't renderable, this should be true
    [[nodiscard]] virtual bool cast_sunlight() const { return true; }
    [[nodiscard]] virtual bool collidable() const { return true; }

    [[nodiscard]] virtual std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const = 0;
};

template<class T>
struct Block : BlockBase {
    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override {
        auto &faces = static_cast<const T *>(this)->faces;

        std::array<std::optional<Face>, 6> res;
        for (int i = 0, j = 0; mask; mask >>= 1, j++)
            if (mask & 1) res[i++] = faces[j];

        return res;
    }
};

struct Air : BlockBase {
    [[nodiscard]] bool renderable() const override { return false; }

    [[nodiscard]] bool fragmentary() const override { return true; }
    [[nodiscard]] bool cast_sunlight() const override { return false; }
    [[nodiscard]] bool collidable() const override { return false; }

    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override { return {}; }
};

struct GrassBlock : Block<GrassBlock> {
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

struct Sand : Block<Sand> {
    const Face faces[6] = {
        {0, 0, 0, 5},
        {0, 1, 0, 5},
        {0, 2, 0, 5},
        {0, 3, 0, 5},
        {0, 4, 0, 5},
        {0, 5, 0, 5}
    };
};

struct Snow : Block<Snow> {
    const Face faces[6] = {
        {0, 0, 0, 6},
        {0, 1, 0, 6},
        {0, 2, 0, 6},
        {0, 3, 0, 6},
        {0, 4, 0, 6},
        {0, 5, 0, 6}
    };
};

struct Podzol : Block<Podzol> {
    const Face faces[6] = {
        {0, 0, 0, 13},
        {0, 1, 0, 13},
        {0, 2, 0, 13},
        {0, 3, 0, 13},
        {0, 4, 0, 12},
        {0, 5, 0, 2}
    };
};

struct Log : Block<Log> {
    const Face faces[6] = {
        {0, 0, 0, 14},
        {0, 1, 0, 14},
        {0, 2, 0, 14},
        {0, 3, 0, 14},
        {0, 4, 0, 15},
        {0, 5, 0, 15}
    };
};

struct Leaf : Block<Leaf> {
    [[nodiscard]] bool fragmentary() const override { return true; }
    const Face faces[6] = {
        {0, 0, 0, 16},
        {0, 1, 0, 16},
        {0, 2, 0, 16},
        {0, 3, 0, 16},
        {0, 4, 0, 16},
        {0, 5, 0, 16}
    };
};

struct PineLog : Block<PineLog> {
    const Face faces[6] = {
        {0, 0, 0, 17},
        {0, 1, 0, 17},
        {0, 2, 0, 17},
        {0, 3, 0, 17},
        {0, 4, 0, 18},
        {0, 5, 0, 18}
    };
};

struct PineLeaf : Block<PineLeaf> {
    [[nodiscard]] bool fragmentary() const override { return true; }
    const Face faces[6] = {
        {0, 0, 0, 19},
        {0, 1, 0, 19},
        {0, 2, 0, 19},
        {0, 3, 0, 19},
        {0, 4, 0, 19},
        {0, 5, 0, 19}
    };
};

template <class T>
struct Decoration : Block<T> {
    [[nodiscard]] bool cast_sunlight() const override { return false; }
};

struct Grass : Decoration<Grass> {
    [[nodiscard]] bool fragmentary() const override { return true; }

    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override {
        return {faces[0], faces[1], faces[2], faces[3]};
    }

    Face faces[4] = {
        {0, 0, 0.5f, 24},
        {0, 1, 0.5f, 24},
        {0, 2, 0.5f, 24},
        {0, 3, 0.5f, 24}
    };
};

struct Rose : Decoration<Rose> {
    [[nodiscard]] bool fragmentary() const override { return true; }

    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override {
        return {faces[0], faces[1], faces[2], faces[3]};
    }

    Face faces[4] = {
        {0, 0, 0.5f, 25},
        {0, 1, 0.5f, 25},
        {0, 2, 0.5f, 25},
        {0, 3, 0.5f, 25}
    };
};

struct Dandelion : Decoration<Dandelion> {
    [[nodiscard]] bool fragmentary() const override { return true; }

    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override {
        return {faces[0], faces[1], faces[2], faces[3]};
    }

    Face faces[4] = {
        {0, 0, 0.5f, 26},
        {0, 1, 0.5f, 26},
        {0, 2, 0.5f, 26},
        {0, 3, 0.5f, 26}
    };
};

struct Shrub : Decoration<Shrub> {
    [[nodiscard]] bool fragmentary() const override { return true; }
    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override {
        return {faces[0], faces[1], faces[2], faces[3]};
    }
    Face faces[4] = {
        {0, 0, 0.5f, 27},
        {0, 1, 0.5f, 27},
        {0, 2, 0.5f, 27},
        {0, 3, 0.5f, 27},
    };
};

struct Water : Block<Water> {
    const Face faces[6] = {
        {0, 0, 0, 120},
        {0, 1, 0, 120},
        {0, 2, 0, 120},
        {0, 3, 0, 120},
        {0, 4, 0, 120},
        {0, 5, 0, 120}
    };
};

}

using block::BlockBase;
BlockBase* get_block(BlockType type);
