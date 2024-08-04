#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec2 uv;

out vec3 fragPos;
out vec2 fragUV;

layout(std140) uniform UBO {
    mat4 proj;
    mat4 view;
    vec3 sunDir;
    float sunI;
};

const float map_size = 3;

void main() {
    vec3 d = normalize(vec3(sunDir.x, sunDir.y - abs(sunDir.x) * 0.08, 0));
    vec3 r = normalize(cross(vec3(0, 1, 0), d));
    vec3 u = normalize(cross(d, r));
    mat3 M = mat3(-r, u, d);
    fragPos = M * (vert - 0.5) + d * 10;
    gl_Position = proj * vec4(mat3(view) * fragPos, 1.0);

    float moon = floor(fract(acos(sunDir.z / .3f) / 3.1416 * 11) * 8 + 1) / map_size;
    vec2 muv = vec2(fract(moon), floor(moon) / map_size);
    fragUV = step(sunI, 0.5) * muv + uv * 0.333;
}