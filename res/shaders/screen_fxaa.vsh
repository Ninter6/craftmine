#version 410

layout (location = 0) in vec2 pos;

void main() {
    gl_position = vec4(pos * 2 - 1, 1);
}