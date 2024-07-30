[[UBO,0]]
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 fragDir;

flat out vec3 camDir;
flat out vec3 camPos;

flat out vec3 sunDir;

layout(std140) uniform UBO {
    mat4 proj;
    mat4 view;
    vec3 sunDir_;
    float sunI;
};

void getCamPosDir() {
    mat3 V = transpose(mat3(view));
    camPos = V * view[3].xyz;
    camDir = V[2];
}

void main() {
    getCamPosDir();
    sunDir = sunDir_;
    fragDir = pos * 2 - 1;
    gl_Position = vec4(fragDir, 1.0);
}