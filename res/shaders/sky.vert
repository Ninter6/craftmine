[[UBO,0]]
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 fragDir;
out vec3 fogCol;

flat out float sunI;
flat out float sunT;

layout(std140) uniform UBO {
    mat4 proj;
    mat4 view;
    vec3 sunDir_;
    float sunI_;
    vec3 fogCol_;
};

void main() {
    sunI = sunI_;
    sunT = (acos(normalize(sunDir_.xy).x) * 0.318 + step(sunI, 0.23)) * 180;
    fragDir = pos - 0.5;
    fogCol = fogCol_;
    gl_Position = proj * vec4(mat3(view) * fragDir, 1.0);
}