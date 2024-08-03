//
// Created by Ninter6 on 2024/7/15.
//

#pragma once

#include <string>
#include <unordered_map>

#include "glad.h"

class Shader;
struct ShaderProcessor {
    std::string process(const std::string& src);

    void bind(Shader* sh);

    std::unordered_map<std::string, int> binding_ubo;
    std::unordered_map<std::string, int> binding_tex;
};

class Shader {
public:
    Shader(const std::string& vs, const std::string& fs);
    ~Shader();

    Shader(Shader&&) = delete;

    void use() const;

    void set_texture(std::string_view name, int tex) const;

    GLuint ID() const {return handle;}

private:
    GLuint handle;

    static void compileShader(GLuint shader, std::string_view source);
};

class SimpleShader : public Shader {
public:
    SimpleShader();

    static std::string vertexShaderSource();
    static std::string fragmentShaderSource();
};

class CubeShader : public Shader {
public:
    CubeShader();

    static std::string vertexShaderSource();
    static std::string fragmentShaderSource();
};

class SkyShader : public Shader {
public:
    SkyShader();

    static std::string vertexShaderSource();
    static std::string fragmentShaderSource();
};

class SpecialShader : public Shader {
public:
    SpecialShader();

    static std::string vertexShaderSource();
    static std::string fragmentShaderSource();
};