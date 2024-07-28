//
// Created by Ninter6 on 2024/7/27.
//

#pragma once

#include "block/block.hpp"
#include "decoration.hpp"

enum BiomeType {
    OCEAN = 0,
    RIVER,
    BEACH,
    DESERT,
    SAVANNA,
    JUNGLE,
    GRASSLAND,
    WOODLAND,
    FOREST,
    RAINFOREST,
    TAIGA,
    TUNDRA,
    ICE,
    MOUNTAIN,

    MAX_BIOME
};

#define MAX_DECORATIONS 8
struct Biome {
    BlockType top_block, bottom_block;
    float roughness, scale, exp;
    Decoration* decorations[MAX_DECORATIONS];
};

Biome biome_data[(size_t)BiomeType::MAX_BIOME] = {
    [OCEAN] = {
        .top_block = BlockType::sand,
        .bottom_block = BlockType::sand,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f
    },
    [RIVER] = {
        .top_block = BlockType::sand,
        .bottom_block = BlockType::sand,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f
    },
    [BEACH] = {
        .top_block = BlockType::sand,
        .bottom_block = BlockType::sand,
        .roughness = 0.2f,
        .scale = 0.8f,
        .exp = 1.3f
    },
    [DESERT] = {
        .top_block = BlockType::sand,
        .bottom_block = BlockType::sand,
        .roughness = 0.6f,
        .scale = 0.6f,
        .exp = 1.2f,
        .decorations = {
            new Shrub(5e-3f)
        }
    },
    [SAVANNA] = {
        .top_block = BlockType::grass_block,
        .bottom_block = BlockType::dirt,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f,
        .decorations = {
            new Oak(1e-3f),
            new Flower(1e-3f),
            new Grass(5e-3f),
        }
    },
    [JUNGLE] = {
        .top_block = BlockType::grass_block,
        .bottom_block = BlockType::dirt,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f,
        .decorations = {
            new Oak(0.01f),
            new Flower(1e-3f),
            new Grass(5e-3f),
        }
    },
    [GRASSLAND] = {
        .top_block = BlockType::grass_block,
        .bottom_block = BlockType::dirt,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f,
        .decorations = {
            new Oak(5e-4f),
            new Flower(3e-3f),
            new Grass(0.02f),
        }
    },
    [WOODLAND] = {
        .top_block = BlockType::grass_block,
        .bottom_block = BlockType::dirt,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f,
        .decorations = {
            new Oak(7e-3f),
            new Flower(3e-3f),
            new Grass(8e-3f),
        }
    },
    [FOREST] = {
        .top_block = BlockType::grass_block,
        .bottom_block = BlockType::dirt,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f,
        .decorations = {
            new Oak(9e-3f),
            new Flower(3e-3f),
            new Grass(8e-3f),
        }
    },
    [RAINFOREST] = {
        .top_block = BlockType::dirt,
        .bottom_block = BlockType::dirt,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f,
        .decorations = {
            new Oak(9e-3f),
            new Flower(3e-3f),
            new Grass(8e-3f),
        }
    },
    [TAIGA] = {
        .top_block = BlockType::podzol, // 灰壤
        .bottom_block = BlockType::dirt,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f,
        .decorations = {
            new Pine(0.006f),
            new Flower(1e-3f),
            new Grass(8e-3f),
        }
    },
    [TUNDRA] = {
        .top_block = BlockType::snow,
        .bottom_block = BlockType::stone,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f,
        .decorations = {
            new Pine(5e-4f)
        }
    },
    [ICE] = {
        .top_block = BlockType::snow,
        .bottom_block = BlockType::stone,
        .roughness = 1.0f,
        .scale = 1.0f,
        .exp = 1.0f
    },
    [MOUNTAIN] = {
        .top_block = BlockType::snow,
        .bottom_block = BlockType::stone,
        .roughness = 2.0f,
        .scale = 1.2f,
        .exp = 1.0f
    },
};

constexpr BiomeType BIOME_TABLE[6][6] = {
    { ICE, TUNDRA, GRASSLAND,   DESERT,     DESERT,     DESERT },
    { ICE, TUNDRA, GRASSLAND,   GRASSLAND,  DESERT,     DESERT },
    { ICE, TUNDRA, WOODLAND,    WOODLAND,   SAVANNA,    SAVANNA },
    { ICE, TUNDRA, TAIGA,       WOODLAND,   SAVANNA,    SAVANNA },
    { ICE, TUNDRA, TAIGA,       FOREST,     JUNGLE,     JUNGLE },
    { ICE, TUNDRA, TAIGA,       TAIGA,      JUNGLE,     JUNGLE }
};

const float HEAT_MAP[] = {
    0.05f,
    0.18f,
    0.4f,
    0.6f,
    0.8f
};

const float MOISTURE_MAP[] = {
    0.2f,
    0.3f,
    0.5f,
    0.6f,
    0.7f
};

// h = height, [-1, 1]
// m = moisture, [0, 1]
// t = temperature [0, 1]
// n = mountain noise [0, 1]
// i = modified heightmap noise [0, 1]
constexpr BiomeType get_biome(float h, float m, float t, float n, float i) {
    if (h <= 0.0f || n <= 0.0f)
        return OCEAN;
    else if (h <= 0.005f)
        return BEACH;

    if (n >= 0.2f && i >= 0.3f)
        return MOUNTAIN;

    size_t t_i = 0, m_i = 0;
    for (; t_i < 4; t_i++)
        if (t <= HEAT_MAP[t_i])
            break;
    for (; m_i < 4; m_i++)
        if (m <= MOISTURE_MAP[m_i])
            break;

    return BIOME_TABLE[m_i][t_i];
}