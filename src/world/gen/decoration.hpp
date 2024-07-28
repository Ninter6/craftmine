//
// Created by Ninter6 on 2024/7/27.
//

#pragma once

#include <random>

class Chunk;
class World;

struct Decoration {
    Decoration(float chance) : chance(chance) {}
    virtual ~Decoration() = default;
    virtual bool apply(World* w, Chunk& c, int seed, int x, int y, int z) const = 0;
    float chance;
};

struct Oak : Decoration {
    using Decoration::Decoration;
    bool apply(World* w, Chunk& c, int seed, int x, int y, int z) const override;
};

struct Pine : Decoration {
    using Decoration::Decoration;
    bool apply(World* w, Chunk& c, int seed, int x, int y, int z) const override;
};

struct Flower : Decoration {
    using Decoration::Decoration;
    bool apply(World* w, Chunk& c, int seed, int x, int y, int z) const override;
};

struct Grass : Decoration {
    using Decoration::Decoration;
    bool apply(World* w, Chunk& c, int seed, int x, int y, int z) const override;
};

struct Shrub : Decoration {
    using Decoration::Decoration;
    bool apply(World* w, Chunk& c, int seed, int x, int y, int z) const override;
};
