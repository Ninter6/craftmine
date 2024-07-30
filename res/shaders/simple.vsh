[[UBO,0]]
// [[name,binding]]
#version 330 core

layout (location = 0) in vec3 vert;
layout (location = 1) in vec2 uv;

layout (location = 2) in vec3 pos;
layout (location = 3) in int facing;
layout (location = 4) in float posOffset;
layout (location = 5) in float texIndex;
layout (location = 6) in float lightIntensity;
layout (location = 7) in vec4 color;

out vec2 fragUV;
flat out vec4 fragColor;

layout(std140) uniform UBO {
    mat4 proj;
    mat4 view;
    vec3 sunDir;
    float sunI;
};

const float map_size = 12;

const mat3 FM[6] = mat3[](
    mat3(
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    ), // front
    mat3(
        0, 0, 1,
        0, 1, 0,
       -1, 0, 0
    ), // left
    mat3(
       -1, 0, 0,
        0, 1, 0,
        0, 0,-1
    ), // back
    mat3(
        0, 0,-1,
        0, 1, 0,
        1, 0, 0
    ), // right
    mat3(
        1, 0, 0,
        0, 0, 1,
        0,-1, 0
    ), // up
    mat3(
        1, 0, 0,
        0, 0,-1,
        0, 1, 0
    ) // down
);
const vec3 N[6] = vec3[6](
    vec3( 0, 0,-1),
    vec3( 1, 0, 0),
    vec3( 0, 0, 1),
    vec3(-1, 0, 0),
    vec3( 0, 1, 0),
    vec3( 0,-1, 0)
);

void main() {
    vec3 p = vec3(vert.xy, vert.z + posOffset);
    vec3 world_pos = FM[facing] * (p-0.5) + 0.5 + pos;
    gl_Position = proj * view * vec4(world_pos, 1.0);

    float g = texIndex / map_size;
    vec2 uvOffset = vec2(fract(g)*map_size, floor(g));
    fragUV = (1.0 - uv + uvOffset) / map_size;

    float S = (max(dot(N[facing], sunDir), 0) * 0.4 + 0.6) * sunI * pow(lightIntensity, sunI);
    fragColor = color * S;
}