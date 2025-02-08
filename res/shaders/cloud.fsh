[{tex, 8}]
#version 410 core

#define CLOUD_HEIGHT 91.2
#define SIGHT_RADIUS (1.5*CHUNK_SIZE_Z*SIGHT_DISTANCE)
const float HB = CLOUD_HEIGHT, HE = CLOUD_HEIGHT + 1;

in vec3 fragPos;
flat in vec3 camPos;
flat in vec4 proj2;
flat in vec4 proj3;

layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;

uniform sampler2D tex;

vec3 ray_cast(vec3 pos, vec3 dir) {
    if (HB <= pos.y && pos.y <= HE)
        return pos + dir * 0.1;

    if (abs(dir.y) < 1E-6) discard;

    float t = (pos.y < HB ? HB-pos.y : HE-pos.y) / dir.y;
    if (t <= 0) discard;

    return pos + dir * t;
}

void main() {
    vec3 camDir = normalize(fragPos - camPos);

    vec4 wpos = vec4(ray_cast(camPos, camDir), 1);
    gl_FragDepth = dot(wpos, proj2) / dot(wpos, proj3) * .5f + .5f;
    float dis = distance(wpos.xz, camPos.xz);
    if (dis > SIGHT_RADIUS || gl_FragDepth >= 1) discard;

    const float max_step = 16;
    const float len = 0.8;
    float a = -1*smoothstep(SIGHT_RADIUS*.9, SIGHT_RADIUS, dis);
    for (int i = 0; i < max_step; i++) {
        vec3 p = wpos.xyz + camDir * i * len;
        if ((p.y - HE)*(p.y - HB) > 0) break;
        a += texture(tex, p.xz / 128).a * 0.07;
    }
    vec4 color = vec4(1, 1, 1, min(a, 1));

    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
    accum = vec4(color.rgb * color.a, color.a) * 3e3;
    reveal = color.a;
}