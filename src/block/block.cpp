//
// Created by Ninter6 on 2024/7/19.
//

#include "block.hpp"
#include "application.hpp"

// BlockBase* blocks[(size_t)BlockType::MAX_BLOCKS] = {
//     new Air,
//     new GrassBlock,
//     new Dirt,
//     new Stone,
//     new Sand,
//     new Snow,
//     new Podzol,
//     new Log,
//     new Leaf,
//     new PineLog,
//     new PineLeaf,
//     new Grass,
//     new Rose,
//     new Dandelion,
//     new Shrub,
//     new Water,
//     new WoodenPlank,
//     new Glass,
//     new ColoredGlass(0), // red
//     new ColoredGlass(1), // green
//     new ColoredGlass(2), // blue
//     new GlassNT,
//     new Torch,
//
//     new Brightness<24>
// };

BlockBase* get_block(BlockType type) {
    return &get_block_material()->get_block((uint8_t)type);
}

BlockBase* get_block(const std::string& name) {
    return &get_block_material()->get_block(name);
}
