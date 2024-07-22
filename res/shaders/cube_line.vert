[[CameraUBO,0]]
#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec2 fragUV;

layout(std140) uniform CameraUBO {
    mat4 proj;
    mat4 view;
};

uniform vec3 position;

void main() {
    vec4 worldPos = vec4(pos + position, 1.0);
    vec4 viewPos = proj * view * worldPos;

    fragUV = (uv - 0.5) * 2;
    gl_Position = vec4(viewPos.xy / viewPos.w, 0.0, 1.0);
}