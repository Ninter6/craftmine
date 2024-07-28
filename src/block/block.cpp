//
// Created by Ninter6 on 2024/7/19.
//

#include "block.hpp"

using namespace block;

BlockBase* blocks[(size_t)BlockType::MAX_BLOCKS] = {
    new Air,
    new GrassBlock,
    new Dirt,
    new Stone,
    new Sand,
    new Snow,
    new Podzol,
    new Log,
    new Leaf,
    new PineLog,
    new PineLeaf,
    new Grass,
    new Rose,
    new Dandelion,
    new Shrub,
    new Water,
};

BlockBase* get_block(BlockType type) {
    return blocks[(size_t)type];
}