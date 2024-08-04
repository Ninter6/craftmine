#version 410 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;

out vec2 fragUV;

//uniform vec2 offset;
//uniform vec2 scale;

void main() {
    fragUV = uv;
    gl_Position = vec4(pos * 2 - 1, 0.0, 1.0);
}