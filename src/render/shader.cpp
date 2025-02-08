//
// Created by Ninter6 on 2024/7/15.
//

#include "shader.hpp"

#include "world/chunk.hpp"
#include "world/world.hpp"

#include <cassert>
#include <fstream>
#include <sstream>

#include "utils/check_gl.h"

#define STR(x) #x
#define XSTR(x) STR(x)

constexpr auto shader_def = "\n"
    "#define CHUNK_SIZE_X " XSTR(CHUNK_SIZE_X) "\n"
    "#define CHUNK_SIZE_Z " XSTR(CHUNK_SIZE_Z) "\n"
    "#define SIGHT_DISTANCE " XSTR(SIGHT_DISTANCE) "\n";

std::string ShaderProcessor::process(const std::string &src) {
    auto lasta = src.rfind("}]");
    auto lastb = src.rfind("]]");
    auto last = std::max(lasta == std::string::npos ? 0 : lasta, lastb == std::string::npos ? 0 : lastb);
    if (last == 0) return std::string{src}.insert(src.find("core") + 5, shader_def);

    assert(src.find("#version") > last && src.find("core") != std::string::npos);

    auto bind = src.substr(0, last + 2);
    auto rst = src.substr(last + 3).insert(src.find("core") - last + 2, shader_def);

    std::stringstream ss;
    ss << bind << "\n";

    std::string line;
    while (std::getline(ss, line)) {
        if (line.compare(0, 2, "[{") == 0) break;
        if (line.compare(0, 2, "[[") != 0) continue;
        auto sp = line.find(',');
        auto name = line.substr(2, sp-2);
        auto num = line.substr(sp+1, line.find("]]"));
        binding_ubo.emplace(name, std::stoi(num));
    }
    do {
        if (line.compare(0, 2, "[{") != 0) continue;
        auto sp = line.find(',');
        auto name = line.substr(2, sp-2);
        auto num = line.substr(sp+1, line.find("}]"));
        binding_tex.emplace(name, std::stoi(num));
    } while (std::getline(ss, line));

    return rst;
}

void ShaderProcessor::bind(Shader* sh) {
    sh->use();
    for (const auto& [name, index] : binding_ubo) {
        auto ubo_id = glGetUniformBlockIndex(sh->ID(), name.c_str());
        CHECK_GL(glUniformBlockBinding(sh->ID(), ubo_id, index));
    }
    for (const auto& [name, index] : binding_tex) {
        auto tex_id = glGetUniformLocation(sh->ID(), name.c_str());
        CHECK_GL(glUniform1i(tex_id, index));
    }
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

    pr.bind(this);
}

void Shader::compileShader(GLuint shader, std::string_view source) {
    auto src = source.data();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

#ifndef NDEBUG
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string errorLog("\0", length);
        glGetShaderInfoLog(shader, length, nullptr, errorLog.data());
        throw std::runtime_error("Failed to compile shader:\n" + errorLog);
    }
#endif
}

void Shader::use() const {
    glUseProgram(handle);
}

void Shader::set_texture(std::string_view name, int tex) const {
    use();
    glUniform1i(glGetUniformLocation(handle, name.data()), tex);
}

Shader::~Shader() {
    glDeleteProgram(handle);
}

SimpleShader::SimpleShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string SimpleShader::vertexShaderSource() {
    std::ifstream fquadv{FILE_ROOT"shaders/simple.vsh"};
    return {
        std::istreambuf_iterator<char>{fquadv},
        std::istreambuf_iterator<char>{}
    };
}

std::string SimpleShader::fragmentShaderSource() {
    std::ifstream fquadf{FILE_ROOT"shaders/simple.fsh"};
    return {
        std::istreambuf_iterator<char>{fquadf},
        std::istreambuf_iterator<char>{}
    };
}

CubeShader::CubeShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string CubeShader::vertexShaderSource() {
    std::ifstream vert{FILE_ROOT"shaders/cube_line.vert"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string CubeShader::fragmentShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/cube_line.frag"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

SkyShader::SkyShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string SkyShader::vertexShaderSource() {
    std::ifstream vert{FILE_ROOT"shaders/sky.vert"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string SkyShader::fragmentShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/sky.frag"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

SunShader::SunShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string SunShader::vertexShaderSource() {
    std::ifstream vert{FILE_ROOT"shaders/sun.vert"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string SunShader::fragmentShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/sun.frag"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

SpecialShader::SpecialShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string SpecialShader::vertexShaderSource() {
    std::ifstream vert{FILE_ROOT"shaders/special.vsh"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string SpecialShader::fragmentShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/special.fsh"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

CompositeShader::CompositeShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string CompositeShader::vertexShaderSource() {
    std::ifstream vert{FILE_ROOT"shaders/composite.vert"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string CompositeShader::fragmentShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/composite.frag"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

UIShader::UIShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string UIShader::vertexShaderSource() {
    std::ifstream vert{FILE_ROOT"shaders/ui.vsh"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string UIShader::fragmentShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/ui.fsh"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

ScreenShader::ScreenShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string ScreenShader::vertexShaderSource() {
    std::ifstream vert{FILE_ROOT"shaders/screen_fxaa.vsh"};
    return {
        std::istreambuf_iterator<char>{vert},
        std::istreambuf_iterator<char>{}
    };
}

std::string ScreenShader::fragmentShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/screen_fxaa.fsh"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

CloudShader::CloudShader()
: Shader(vertexShaderSource(), fragmentShaderSource()) {}

std::string CloudShader::vertexShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/cloud.vsh"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}

std::string CloudShader::fragmentShaderSource() {
    std::ifstream frag{FILE_ROOT"shaders/cloud.fsh"};
    return {
        std::istreambuf_iterator<char>{frag},
        std::istreambuf_iterator<char>{}
    };
}
