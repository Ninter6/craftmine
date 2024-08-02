[[UBO,0]]
#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec2 fragUV;

layout(std140) uniform UBO {
    mat4 proj;
    mat4 view;
};

uniform vec3 position;

void main() {
    vec4 worldPos = vec4(pos + position, 1.0);
    gl_Position = proj * view * worldPos;

    fragUV = uv * 2 - 1;
}