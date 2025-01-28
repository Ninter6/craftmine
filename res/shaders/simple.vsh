[[UBO,0]]
// [[name,binding]]
#version 330 core

layout (location = 0) in vec3 vert;
layout (location = 1) in vec2 uv;

layout (location = 2) in vec3 pos;
layout (location = 3) in int facing;
layout (location = 4) in float posOffset;
layout (location = 5) in float texIndex;
layout (location = 6) in float sunIntensity;
layout (location = 7) in float lightIntensity;
layout (location = 8) in vec4 color;

out vec2 fragUV;
flat out vec4 fragColor;
flat out vec3 fogCol;
out float fogDen;

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

float fogDensity(vec3 camPos, vec3 fragPos) {
    const float S = CHUNK_SIZE_X * (SIGHT_DISTANCE - 1);
    const float E = CHUNK_SIZE_X * (SIGHT_DISTANCE + 1);
    return smoothstep(S, E, distance(camPos.xz, fragPos.xz));
}

vec3 fogColor(vec3 cam, vec3 dir, float I) {
    float t = max(dot(vec2(0.995, 0.0995),-dir.xy), dot(vec2(0.9982,-0.05995), dir.xy));
    vec3 color = (I * 0.8 + 0.1) * mix(vec3(0.6, 0.7, 0.9), vec3(0.7, 0.2, 0.1), pow(t * 0.5 + 0.5, 128));
    vec3 fog_col = pow(pow(dot(-dir, cam), 2) * 0.2 + 0.8, 3) * color;
    return fog_col;
}

void main() {
    vec3 p = vec3(vert.xy, vert.z + posOffset);
    vec3 fragPos = FM[facing] * (p-0.5) + 0.5 + pos;
    gl_Position = proj * view * vec4(fragPos, 1.0);

    mat3 V = transpose(mat3(view));
    vec3 camPos = -V * view[3].xyz;
    vec3 camDir = V[2];

    fogCol = fogColor(camDir, normalize(vec3(sunDir.x, sunDir.y - abs(sunDir.x) * 0.08, 0)), sunI);
    fogDen = fogDensity(camPos, fragPos);

    float g = texIndex / map_size;
    vec2 uvOffset = vec2(fract(g)*map_size, floor(g));
    fragUV = (1.0 - uv + uvOffset) / map_size;

    float sl = max(dot(N[facing], sunDir), 0) * 0.4 + 0.6;
    float I = max(sl * sunI * sunIntensity, lightIntensity) + 0.157f;
    fragColor = vec4(color.rgb * I, color.a);
}