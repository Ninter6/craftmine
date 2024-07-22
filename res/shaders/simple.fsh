#version 330 core
in vec2 fragUV;
in vec4 fragColor;

out vec4 outColor;

uniform sampler2D tex;

void main() {
    vec4 color = texture(tex, fragUV);
    if (color.a < 1e-7) discard;
    outColor = color * fragColor;
}