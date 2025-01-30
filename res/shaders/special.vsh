#version 410 core

layout (location = 0) in vec3 vert;
layout (location = 1) in vec2 uv;

layout (location = 2) in vec3 pos;
layout (location = 3) in int facing;
layout (location = 4) in float posOffset;
layout (location = 5) in float sunIntensity;
layout (location = 6) in vec4 color;
layout (location = 7) in float firstTex;
layout (location = 8) in float texLength;
layout (location = 9) in float remain;
layout (location = 10)in float lightIntensity;

out vec2 fuv;
out vec2 cuv;
flat out float tex_t;
flat out vec4 fragColor;

layout(std140) uniform UBO {
    mat4 proj;
    mat4 view;
    vec3 sunDir;
    float sunI;
    vec3 fogCol;
};

const float map_size = 12;
const float tick_per_half_day = 6000;

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

void calcu_uv() {
    float t = fract(acos(normalize(sunDir.xy).x) / 3.141593f * tick_per_half_day / remain);
    float g = mix(firstTex, firstTex + texLength, t);
    float fg = floor(g);
    float cg = ceil(g) > (firstTex + texLength - 1) ? firstTex : ceil(g);
    tex_t = g - fg;
    vec2 uv1 = vec2(fract(fg / map_size)*map_size, floor(fg / map_size));
    vec2 uv2 = vec2(fract(cg / map_size)*map_size, floor(cg / map_size));
    fuv = (1.0 - uv + uv1) / map_size;
    cuv = (1.0 - uv + uv2) / map_size;
}

void main() {
    vec3 p = vec3(vert.xy, vert.z + posOffset);
    vec3 world_pos = FM[facing] * (p-0.5) + 0.5 + pos;
    gl_Position = proj * view * vec4(world_pos, 1.0);

    calcu_uv();

    float sl = max(dot(N[facing], sunDir), 0) * 0.4 + 0.6;
    float I = max(sl * sunI * sunIntensity, lightIntensity) + 0.157f;
    fragColor = vec4(color.rgb * I, color.a);
}