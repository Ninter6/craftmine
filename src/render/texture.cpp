//
// Created by Ninter6 on 2024/7/18.
//

#include "texture.hpp"

#include "stb_image.h"

std::unique_ptr<Texture> Texture::LoadFromFile(std::string_view file) {
    int width, height, channels;
//    stbi_set_flip_vertically_on_load(true); // Flip image vertically
    auto data = stbi_load(file.data(), &width, &height, &channels, 4);

    if (!data) {
        throw std::runtime_error("Error loading texture from file: " + std::string(file));
    }

    auto rst = std::make_unique<Texture>(data, mathpls::ivec2{width, height});

    stbi_image_free(data);

    return rst;
}

Texture::Texture(uint8_t* data, mathpls::ivec2 size) : size(size) {
    glGenTextures(1, &id);
    bind();

    // No interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::bind(int n) const {
    glActiveTexture(GL_TEXTURE0 + n);
    bind();
}
