//
// Created by Ninter6 on 2024/7/19.
//

#pragma once

#include <bit>
#include <array>
#include <optional>

#include "math/mathpls.h"
#include "render/render_type.hpp"

struct BasicBlock {
    std::string name;

    bool renderable = true;

    bool fragmentary = false;
    bool cast_light = true;
    bool collidable = true;

    // bool is_special = false;
    bool transparent = false;
    bool fluid = false;

    static constexpr float max_emission = 25;
    int16_t emission = 0;

    std::vector<Face> common_faces;
    std::vector<SpecialFace> special_faces;
};

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
    glass,
    glass_red,
    glass_green,
    glass_blue,
    glass_nt,
    torch,

    brightness,

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
    [[nodiscard]] virtual bool cast_light() const { return true; }
    [[nodiscard]] virtual bool collidable() const { return true; }

    [[nodiscard]] virtual bool special() const { return false; }
    [[nodiscard]] virtual bool transparent() const { return false; }
    [[nodiscard]] virtual bool fluid() const { return false; }

    static constexpr float max_emission = 25;
    [[nodiscard]] virtual int emission() const { return 0; }

    [[nodiscard]] virtual std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const = 0;
    [[nodiscard]] virtual std::array<std::optional<SpecialFace>, 6> get_special_faces(FaceMask mask) const = 0;
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
    [[nodiscard]] std::array<std::optional<SpecialFace>, 6> get_special_faces(FaceMask mask) const override {
        return {};
    }
};

template <class T>
struct SpecialBlock : BlockBase {
    [[nodiscard]] bool special() const override { return true; }
    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override {
        return {};
    }
    [[nodiscard]] std::array<std::optional<SpecialFace>, 6> get_special_faces(FaceMask mask) const override {
        auto &faces = static_cast<const T *>(this)->faces;

        std::array<std::optional<SpecialFace>, 6> res;
        for (int i = 0, j = 0; mask; mask >>= 1, j++)
            if (mask & 1) res[i++] = faces[j];

        return res;
    }
};

struct Air : BlockBase {
    [[nodiscard]] bool renderable() const override { return false; }

    [[nodiscard]] bool fragmentary() const override { return true; }
    [[nodiscard]] bool cast_light() const override { return false; }
    [[nodiscard]] bool collidable() const override { return false; }

    [[nodiscard]] std::array<std::optional<Face>, 6> get_faces(FaceMask mask) const override { return {}; }
    [[nodiscard]] std::array<std::optional<SpecialFace>, 6> get_special_faces(FaceMask mask) const override { return {}; }
};

template <int E>
struct Brightness : Air {
    [[nodiscard]] int emission() const override { return E; }
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
    [[nodiscard]] bool fragmentary() const override { return true; }
    [[nodiscard]] bool cast_light() const override { return false; }
};

struct Grass : Decoration<Grass> {
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

struct Water : SpecialBlock<Water> {
    [[nodiscard]] bool transparent() const override { return true; }
    [[nodiscard]] bool fluid() const override { return true; }
    const SpecialFace faces[6] = {
        {.facing = 0, .firstTex = 120, .texLength = 6, .remainTick = 100},
        {.facing = 1, .firstTex = 120, .texLength = 6, .remainTick = 100},
        {.facing = 2, .firstTex = 120, .texLength = 6, .remainTick = 100},
        {.facing = 3, .firstTex = 120, .texLength = 6, .remainTick = 100},
        {.facing = 4, .firstTex = 120, .texLength = 6, .remainTick = 100},
        {.facing = 5, .firstTex = 120, .texLength = 6, .remainTick = 100}
    };
};

struct WoodenPlank : Block<WoodenPlank> {
    const Face faces[6] = {
        {0, 0, 0, 20},
        {0, 1, 0, 20},
        {0, 2, 0, 20},
        {0, 3, 0, 20},
        {0, 4, 0, 20},
        {0, 5, 0, 20}
    };
};

struct Glass : Block<Glass> {
    [[nodiscard]] bool fragmentary() const override { return true; }
    [[nodiscard]] bool cast_light() const override { return false; }
    const Face faces[6] = {
        {0, 0, 0, 36},
        {0, 1, 0, 36},
        {0, 2, 0, 36},
        {0, 3, 0, 36},
        {0, 4, 0, 36},
        {0, 5, 0, 36}
    };
};

struct ColoredGlass : SpecialBlock<ColoredGlass> {
    [[nodiscard]] bool transparent() const override { return true; }
    explicit ColoredGlass(int c) : faces{
        {.facing = 0, .firstTex = 37 + (float)c},
        {.facing = 1, .firstTex = 37 + (float)c},
        {.facing = 2, .firstTex = 37 + (float)c},
        {.facing = 3, .firstTex = 37 + (float)c},
        {.facing = 4, .firstTex = 37 + (float)c},
        {.facing = 5, .firstTex = 37 + (float)c}
    } {}
    SpecialFace faces[6];
};

struct GlassNT : SpecialBlock<GlassNT> {
    [[nodiscard]] bool transparent() const override { return true; }
    [[nodiscard]] bool cast_light() const override { return false; }
    const SpecialFace faces[6] = {
        {.facing = 0, .firstTex = 37, .texLength = 3, .remainTick = 100},
        {.facing = 1, .firstTex = 37, .texLength = 3, .remainTick = 100},
        {.facing = 2, .firstTex = 37, .texLength = 3, .remainTick = 100},
        {.facing = 3, .firstTex = 37, .texLength = 3, .remainTick = 100},
        {.facing = 4, .firstTex = 37, .texLength = 3, .remainTick = 100},
        {.facing = 5, .firstTex = 37, .texLength = 3, .remainTick = 100}
    };
};

struct Torch : Decoration<Torch> {
    [[nodiscard]] int emission() const override { return 15; }
    const Face faces[6] = {
        {0, 0, .375f, 28},
        {0, 1, .375f, 28},
        {0, 2, .375f, 28},
        {0, 3, .375f, 28},
        {0, 4, .25f, 29},
        {0, 5, 0, 29}
    };
};

}

using BlockBase = const BasicBlock;
BlockBase* get_block(BlockType type);
BlockBase* get_block(const std::string& name);
