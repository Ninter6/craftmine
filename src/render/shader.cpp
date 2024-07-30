//
// Created by Ninter6 on 2024/7/15.
//

#include "shader.hpp"

#include <fstream>
#include <sstream>

std::string ShaderProcessor::process(const std::string &src) {
    auto last = src.rfind("]]");
    if (last == std::string::npos) return src;

    auto bind = src.substr(0, last + 2);
    auto rst = src.substr(last + 2);

    std::stringstream ss;
    ss << bind << "\n";

    std::string line;
    while (std::getline(ss, line)) {
        if (line.compare(0, 2, "[[") != 0) continue;
        auto sp = line.find(',');
        auto name = line.substr(2, sp-2);
        auto num = line.substr(sp+1, line.find("]]"));
        bindings.emplace(name, std::stoi(num));
    }

    return rst;
}

Shader::Shader(const std::string& vs, const std::string& fs) {
    ShaderProcessor pr{};

    auto vertex = glCreateShader(GL_VERTEX_SHADER);
    compileShader(vertex, pr.process(vs));

    auto fragment = glCreateShader(GL_FRAGMENT_SHADER);
    compileShader(fragment, pr.process(fs));

    handle = glCreateProgram();
    glAttachShader(handle, vertex);
    glAttachShader(handle, fragment);
    glLinkProgram(handle);

    GLint success;
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if(!success) {
        GLint length;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
        std::string infoLog("\0", length);
        glGetProgramInfoLog(handle, 512, nullptr, infoLog.data());
        throw std::runtime_error("Failed to link shader:\n" + infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    processBindings(pr.bindings);
}

void Shader::processBindings(const std::unordered_map<std::string, int>& bindings) {
    for (const auto& [name, index] : bindings) {
        unsigned int ubo_id = glGetUniformBlockIndex(handle, name.c_str());
        glUniformBlockBinding(handle, ubo_id, index);
    }
}

void Shader::compileShader(GLuint shader, std::string_view source) {
    auto src = source.data();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string errorLog("\0", length);
        glGetShaderInfoLog(shader, length, nullptr, errorLog.data());
        throw std::runtime_error("Failed to compile shader:\n" + errorLog);
    }
}

void Shader::use() const {
    glUseProgram(handle);
}

void Shader::set_texture(std::string_view name, int tex) const {
    glUniform1i(glGetUniformLocation(handle, name.data()), tex);
}

Shader::~Shader() {
    glDeleteProgram(handle);
}

SimpleShader::SimpleShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string SimpleShader::vertexShaderSource() {
    std::ifstream fquadv{"/Users/mac/Desktop/temp/craftmine/res/shaders/simple.vsh"};
    return {
        std::istreambuf_iterator<char>{fquadv},
        std::istreambuf_iterator<char>{}
    };
}

std::string SimpleShader::fragmentShaderSource() {
    std::ifstream fquadf{"/Users/mac/Desktop/temp/craftmine/res/shaders/simple.fsh"};
    return {
        std::istreambuf_iterator<char>{fquadf},
        std::istreambuf_iterator<char>{}
    };
}

CubeShader::CubeShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string CubeShader::vertexShaderSource() {
    std::ifstream vert{"/Users/mac/Desktop/temp/craftmine/res/shaders/cube_line.vert"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string CubeShader::fragmentShaderSource() {
    std::ifstream frag{"/Users/mac/Desktop/temp/craftmine/res/shaders/cube_line.frag"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

SkyShader::SkyShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string SkyShader::vertexShaderSource() {
    std::ifstream vert{"/Users/mac/Desktop/temp/craftmine/res/shaders/sky.vert"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string SkyShader::fragmentShaderSource() {
    std::ifstream frag{"/Users/mac/Desktop/temp/craftmine/res/shaders/sky.frag"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}