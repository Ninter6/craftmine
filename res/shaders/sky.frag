#version 410 core

in vec3 fragDir;

flat in vec3 camDir;
flat in vec3 camPos;

flat in vec3 sunDir;
flat in float sunI;

out vec4 color;

uniform sampler2D sky;
uniform sampler2D star;

vec2 get_uv(vec3 f_dir) {
    float cy = sqrt(1-f_dir.y*f_dir.y);
    vec2 uv;
    uv.y = asin(f_dir.y) / 1.58;
    uv.x = atan(f_dir.x / cy, f_dir.z / cy) / 6.3 + 0.5;
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
    vec4 col = texture(star, vec2(uv.x + t/1919.810, uv.y)) * max((0.5 - sunI) / 0.3, 0);
    return mix(vec4(0.012, 0.02, 0.04, 1.0), col, smoothstep(0, 0.2, uv.y));
}

void main() {
    vec3 f_dir = normalize(fragDir);
    vec3 sun_dir = normalize(vec3(sunDir.xy, 0));

    vec2 uv = get_uv(f_dir);
    vec2 l_uv = vec2(uv.x, max(uv.y, 0));

    float sun_t = acos(sun_dir.x) + 3.14159 * step(sunI, 0.5);
    sun_t *= 180.0 / 3.14159;

    color = calcu_sky(l_uv, sun_t) + calcu_star(uv, sun_t);
}