//
// Created by Ninter6 on 2024/8/19.
//

#include <mutex>
#include <thread>
#include <cassert>
#include <fstream>
#include <algorithm>
#include <chrono>

#include "worldsaver.hpp"
#include "world/world.hpp"

#include "application.hpp"

std::string worldname2filename(const std::string& wn) {
    return FILE_ROOT"world/" + wn + ".cmw";
}

WorldSaver::WorldSaver(std::string_view filename) : filename(filename) {}

struct ReadChunk {
    ChunkPos pos{};
    std::vector<BlockType> data{};
};

void WorldSaver::load(World& world) {
    std::ifstream file(filename);
    if (!file.good()) {
        printf("ERROR: failed to open file: %s", filename.c_str());
        std::terminate();
    }

    // world information
    file >> world.name >> world.seed;

    auto& cam = *world.cam;
    // camera position
    file >> cam.position.x
         >> cam.position.y
         >> cam.position.z;

    // camera forward
    file >> cam.forward.x
         >> cam.forward.y
         >> cam.forward.z;

    // asynchronous testing
    int nc{};
    file >> nc;
    std::vector<ReadChunk> rc(nc);
    for (auto&& [pos, data] : rc) {
        int size{};
        file >> pos.x >> pos.z >> size;
        data.resize(size);
        file.ignore(1);
        file.read((char*)data.data(), size);
    }
    // reading must be synchronized
    // reading completed

    // the following code is executed asynchronously
    constexpr int nt = 8;
    std::atomic_int c = 0, s = 0;
    std::mutex mut;
    std::vector<Chunk> chunks; chunks.reserve(nc);
    std::vector<std::pair<mathpls::ivec3, BlockType>> lsb; // light source block
    for (int n = 0; n < nt; ++n)
        async([&, n]{
            const auto begin = nc*n/nt;
            const auto end = std::min(nc*(n+1)/nt, nc);
            std::vector<std::pair<mathpls::ivec3, BlockType>> ll;
            std::vector<Chunk> cs; cs.reserve(end - begin);
            for (int i = begin; i < end; ++i, ++c) {
                auto& [pos, data] = rc[i];
                Chunk& chunk = cs.emplace_back(pos);
                chunk.raise_height((float)data.size() / 256.f);
                for (int y = 0, b = 0; b < data.size(); y++)
                    for (int z = 0; z < 16 && b < data.size(); z++)
                        for (int x = 0; x < 16 && b < data.size(); x++, b++)
                            if (get_block(data[b])->emission > 1)
                                ll.emplace_back(mathpls::ivec3{pos.x + x, y, pos.z + z}, data[b]);
                            else
                                chunk.blocks[y][z][x].type = data[b];
                chunk.update_height_map();
            }
            std::lock_guard lock{mut};
            std::copy(ll.begin(), ll.end(), std::back_inserter(lsb));
            std::copy(cs.begin(), cs.end(), std::back_inserter(chunks));
            ++s;
        });
    auto t0 = std::chrono::system_clock::now();
    do {std::this_thread::yield();
        printf("\r正在加载: %d%%", 100*c/nc);
    } while (s < nt);
    printf("\n区块: %d 光源: %d\n", nc, (int)lsb.size());
    auto t1 = std::chrono::system_clock::now();
    for (auto&& ch : chunks)
        world.new_chunk(ch.position, std::move(ch));
    auto t2 = std::chrono::system_clock::now();
    for (auto&& [p, b] : lsb)
        world.set_block(p, b);
    auto t3 = std::chrono::system_clock::now();
    printf("loading: %.2fs\n",
           std::chrono::duration<double>(t1 - t0).count());
    printf("new_chunk: %.2fs (per chunk: %.2fms)\n",
           std::chrono::duration<double>(t2 - t1).count(),
           std::chrono::duration<double>(t2 - t1).count() / nc * 1e3f);
    printf("set_block: %.2fs (per light: %.2fms)\n",
           std::chrono::duration<double>(t3 - t2).count(),
           std::chrono::duration<double>(t3 - t2).count() / lsb.size() * 1e3f);
}

void WorldSaver::save(const World& world) {
    std::ofstream file(filename);
    assert(file.good());

    // world information
    file << world.name << '\n'
         << world.seed << '\n'
    // camera position
         << world.cam->position.x << ' '
         << world.cam->position.y << ' '
         << world.cam->position.z << '\n'
    // camera forward
         << world.cam->forward.x << ' '
         << world.cam->forward.y << ' '
         << world.cam->forward.z << '\n';

    // chunk data
    file << world.map.size() << '\n';
    for (auto&& [pos, chunk] : world.map) {
        int size = (int)chunk.blocks.size() * 16 * 16;
        uint8_t buf[16 * 16 * 256]{};
        std::transform(&chunk.blocks[0][0][0],
                       &chunk.blocks[0][0][0] + size,
                       buf, [](auto&& b) {
            return (uint8_t)b.type;
        });
        while (size > 0 && buf[size - 1] == (uint8_t)BlockType::air) size--;

        // chunk information
        file << pos.x << ' '
             << pos.z << ' '
             << size << '\n';
        // block data
        file.write((const char*)buf, size);
        file.put('\n');
    }
}