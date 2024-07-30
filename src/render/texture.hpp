//
// Created by Ninter6 on 2024/7/18.
//

#pragma once

#include <memory>
#include <string>

#include "math/mathpls.h"

#include "glad.h"

struct Texture {
    Texture() = default;
    Texture(uint8_t* data, mathpls::ivec2 size, bool linear = false);

    ~Texture();

    Texture(Texture&&) = delete;

    void bind() const;
    void bind(int n) const;

    mathpls::ivec2 size{};
    GLuint id{};

    static std::unique_ptr<Texture> LoadFromFile(std::string_view file, bool linear = false);
};

struct TextureOneColor : public Texture {
    TextureOneColor(mathpls::vec4 col) :
    Texture(mathpls::vec<uint8_t, 4>(mathpls::clamp(round(col*255.f), 0.f, 255.f)).value_ptr(), {1, 1}) {}
};