//
// Created by Ninter6 on 2024/7/19.
//

#include "block.hpp"

BlockBase* blocks[(size_t)BlockType::NUM_BLOCKS] = {
    new Air,
    new Grass,
    new Dirt,
    new Stone,

};

BlockBase* get_block(BlockType type) {
    return blocks[(size_t)type];
}