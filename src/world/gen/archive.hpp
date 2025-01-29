//
// Created by Ninter6 on 2024/8/19.
//

#pragma once

#include <string>

class World;

class Archive {
public:
    explicit Archive(std::string_view filename);

    void load(World& w);
    void save(const World& w);

private:
    std::string filename;
};

std::string worldname2filename(const std::string& wn);