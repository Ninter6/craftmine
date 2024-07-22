//
// Created by Ninter6 on 2024/7/15.
//

#pragma once

#include <string>
#include <unordered_map>

#include "glad.h"

struct ShaderProcessor {
    std::string process(const std::string& src);

    std::unordered_map<std::string, int> bindings;
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

    void processBindings(const std::unordered_map<std::string, int>& bindings);

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