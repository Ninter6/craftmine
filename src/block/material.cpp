//
// Created by Ninter6 on 2025/1/26.
//

#include "material.hpp"

#include <cstdio>
#include <cassert>

constexpr size_t str_hash(std::string_view str) {
    size_t hash_value = 14695981039346656037u;
    for (auto&& c : str) {
        constexpr size_t prime2 = 0x5deece66d;
        constexpr size_t prime1 = 0x9e3779b9;
        hash_value ^= c;
        hash_value *= prime1;
        hash_value = (hash_value << 31) | (hash_value >> (sizeof(hash_value) * 8 - 31));
        hash_value *= prime2;
    }
    return hash_value;
}

bool valuable(std::string_view line) {
    auto pos = line.find_first_not_of(" \t\r");
    return (pos != std::string::npos ? line.length() - pos : line.length()) > 2 && line.substr(pos, 2) != "//";
}

std::pair<std::string, std::string> split_kv(const std::string& line) {
    auto pos = line.find(':');
    auto ab = line.find_first_not_of(" \t\r"), ae = line.find_last_not_of(" \t\r" , (pos != std::string::npos ? pos : line.rfind("//")) - 1) + 1;
    size_t bb = 0, be = 0;
    if (pos != std::string::npos && pos + 1 != line.size())
        bb = line.find_first_not_of(" \t\r", pos + 1), be = line.find_last_not_of(" \t\r", line.rfind("//")) + 1;
    return {
        line.substr(ab, ae - ab),
        line.substr(bb, be - bb)
    };
}

bool BlockMaterial::contains(const std::string& name) const {
    return table.contains(name);
}

const BasicBlock& BlockMaterial::get_block(const std::string& name) const {
    assert(table.contains(name));
    return table.at(name);
}

[[nodiscard]] const BasicBlock& BlockMaterial::get_block(uint8_t id) const {
    assert(id < table.size());
    return table.begin()[id].second;
}

[[nodiscard]] uint8_t BlockMaterial::get_block_id(const std::string& name) const {
    const auto it = table.find(name);
    assert(it != table.end());
    return (uint8_t)(it - table.begin());
}

BlockMaterial::BlockMaterial(std::string_view config) : file(config) {
    std::ifstream ifs{config};

    std::vector<std::string> vb;

    // int count = 0;
    std::string line;
    while (std::getline(ifs, line)) {
        if (!valuable(line)) continue;
        auto [key, value] = split_kv(line);
        if (auto pos = key.find('('); pos != std::string::npos) {
            auto name = key.substr(pos + 1, key.rfind(')') - pos - 1);
            if (!value.empty()) {
                if (value == "__virtual") vb.emplace_back(name);
                else table[name] = get_block(value);
            }
            parse_block(name, ifs);
        } else assert(!"WTF with your material");
    }
    for (auto&& n : vb) table.rotate_pop(n);
}

void BlockMaterial::parse_block(const std::string& name, std::istream& ifs) {
    auto& block = table[name];
    block.name = name;
    std::string line;
    while (std::getline(ifs, line)) {
        if (!valuable(line)) continue;
        auto [k, v] = split_kv(line);
        if (k.find("end_block()") != std::string::npos)
            break;
        if (v.empty()) continue;
        [[maybe_unused]] int facing = -1;
        switch (str_hash(k)) {
            case str_hash("renderable"):
                block.renderable = (v[0] != 'f' && v[0] != '0');
                break;
            case str_hash("fragmentary"):
                block.fragmentary = (v[0] != 'f' && v[0] != '0');
                break;
            case str_hash("cast_light"):
                block.cast_light = (v[0] != 'f' && v[0]!= '0');
                break;
            case str_hash("collidable"):
                block.collidable = (v[0] != 'f' && v[0]!= '0');
                break;
            // case str_hash("is_special"):
            //     block.is_special = (v[0]!= 'f' && v[0]!= '0');
            //     break;
            case str_hash("transparent"):
                block.transparent = (v[0]!= 'f' && v[0]!= '0');
                break;
            case str_hash("fluid"):
                block.fluid = (v[0]!= 'f' && v[0]!= '0');
                break;
            case str_hash("emission"):
                block.emission = (int16_t)std::stoi(v);
                break;
            case str_hash("face[0]"): facing = 0; break;
            case str_hash("face[1]"): facing = 1; break;
            case str_hash("face[2]"): facing = 2; break;
            case str_hash("face[3]"): facing = 3; break;
            case str_hash("face[4]"): facing = 4; break;
            case str_hash("face[5]"): facing = 5; break;
            default: assert(!"WTF with your block");
        }
        if (facing == -1) continue;
        if (v[0] == '(') { // common
            auto& face = block.common_faces.emplace_back();
            face.facing = facing;
            std::sscanf(v.c_str(), "(%f, %f, %x)" , &face.texIndex, &face.posOffset, &face.color);
        } else { // special
            auto& face = block.special_faces.emplace_back();
            face.facing = facing;
            std::sscanf(v.c_str(), "{%f, %f, %f, %f, %x}", &face.firstTex, &face.texLength, &face.remainTick, &face.posOffset, &face.color);
        }
    }
}
