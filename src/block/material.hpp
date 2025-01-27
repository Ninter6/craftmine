//
// Created by Ninter6 on 2025/1/26.
//

#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "container/dense_map.hpp"

#include "block.hpp"

/*
 * A example block table:
 *
 * begin_block(air) // name of the block
 *
 * renderable: false
 *
 * fragmentary: true
 * cast_light: false
 * collidable: false
 *
 * is_special: false
 * transparent: false
 * fluid: false
 *
 * emission: 0
 *
 * face[0]: (0, 0, 0xffffff00) // face[(facing)]: ((tex_index), (offset), (color_u32)) - common face, empty to use default
 * face[1]: {0, 1, 114, 0, 0xffffff00} // face[(facing)]: {(first_tex), (tex_len), (remain_tick), (offset), (color_u32)} - specific face
 *
 * end_block()
 *
 */

struct BlockMaterial {
    BlockMaterial() = default;
    explicit BlockMaterial(std::string_view config);

    void parse_block(const std::string& name, std::istream& context);

    [[nodiscard]] bool contains(const std::string& name) const;
    [[nodiscard]] const BasicBlock& get_block(const std::string& name) const;
    [[nodiscard]] const BasicBlock& get_block(uint8_t id) const;
    [[nodiscard]] uint8_t get_block_id(const std::string& name) const;

    std::string file;
    dense_map<std::string, BasicBlock> table;
};
