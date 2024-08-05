#version 410 core

layout (location = 0) in vec2 vert;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec2 pos;
layout (location = 3) in vec2 scale;
layout (location = 4) in float priority;
layout (location = 5) in float texIndex;

out vec2 fragUV;

uniform float aspect;

const float map_size = 12;

void main() {
    vec2 p = pos + vert * scale * vec2(1, aspect);
    gl_Position = vec4(p / priority, 0.0, 1.0 / priority);

    float g = texIndex / map_size;
    vec2 uvOffset = vec2(fract(g)*map_size, floor(g));
    fragUV = (1.0 - uv + uvOffset) / map_size;
}