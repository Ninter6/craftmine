#version 410 core

in vec2 fuv;
in vec2 cuv;
flat in float tex_t;
flat in vec4 fragColor;

out vec4 outColor;

uniform sampler2D tex;

void main() {
    vec4 color = mix(texture(tex, fuv), texture(tex, cuv), tex_t);
    outColor = color * fragColor;
}