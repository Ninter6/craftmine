[[UBO,0]]
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 fragDir;
out vec3 fogCol;

flat out float sunI;
flat out float sunT;

layout(std140) uniform UBO {
    mat4 proj;
    mat4 view;
    vec3 sunDir_;
    float sunI_;
};

void main() {
    sunI = sunI_;
    sunT = (acos(normalize(sunDir_.xy).x) * 0.318 + step(sunI, 0.23)) * 180;
    fragDir = pos - 0.5;
    gl_Position = proj * vec4(mat3(view) * fragDir, 1.0);

    vec3 cam = transpose(mat3(view))[2];
    vec3 dir = normalize(vec3(sunDir_.x, sunDir_.y - abs(sunDir_.x) * 0.08, 0));
    float t = max(dot(vec2(0.995, 0.0995),-dir.xy), dot(vec2(0.9982,-0.05995), dir.xy));
    vec3 color = (sunI * 0.8 + 0.1) * mix(vec3(0.6, 0.7, 0.9), vec3(0.7, 0.2, 0.1), pow(t * 0.5 + 0.5, 128));
    fogCol = pow(pow(dot(-dir, cam), 2) * 0.2 + 0.8, 3) * color;
}