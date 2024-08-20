//
// Created by Ninter6 on 2024/8/19.
//

#pragma once

#include <string>

class World;

class WorldSaver {
public:
    WorldSaver(std::string_view filename);

    void load(World& w);
    void save(const World& w);

private:
    std::string filename;
};

std::string worldname2filename(const std::string& wn);