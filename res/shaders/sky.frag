[{sky,1}]
[{star,2}]
#version 410 core

in vec3 fragDir;
in vec3 fogCol;

flat in float sunI;
flat in float sunT;

out vec4 color;

uniform sampler2D sky;
uniform sampler2D star;

vec2 get_uv(vec3 f_dir) {
    float cy = sqrt(1-f_dir.y*f_dir.y);
    vec2 uv;
    uv.y = asin(f_dir.y) / 1.58;
    uv.x = 0.5 - atan(f_dir.x / cy, f_dir.z / cy) / 6.3;
    return uv;
}

vec4 calcu_sky(vec2 l_uv, float sun_t) {
    if (195 < sun_t && sun_t < 345)
        return vec4(0, 0, 0, 1);

    sun_t -= step(195, sun_t) * 360;
    sun_t = (sun_t + 15.0) / 210.0 * 512.0;
    float f = floor(sun_t);
    float c = ceil(sun_t) * step(sun_t, 511);
    float t = sun_t - f;

    vec2 f_uv = vec2(fract(f / 16), floor(f / 16) / 32) + vec2(l_uv.x / 16, l_uv.y / 32);
    vec2 c_uv = vec2(fract(c / 16), floor(c / 16) / 32) + vec2(l_uv.x / 16, l_uv.y / 32);

    return mix(texture(sky, f_uv), texture(sky, c_uv), t);
}

vec4 calcu_star(vec2 uv, float t) {
    if (sunI > 0.23) return vec4(0);
    return texture(star, vec2(uv.x + t/1919.810, uv.y)) * max(1 - sunI * 5, 0);
}

float height_fog(vec3 dir) {
    const float S =.1919;
    float E = max(-.810,-exp(-45.14*pow(sunI-.36, 1.2)));
    return smoothstep(S, E, dir.y);
}

void main() {
    vec3 f_dir = normalize(fragDir);

    vec2 uv = get_uv(f_dir);
    vec2 l_uv = vec2(uv.x, max(uv.y, 0));

    color = mix(calcu_sky(l_uv, sunT) + calcu_star(uv, sunT), vec4(fogCol, 1), height_fog(f_dir));
}