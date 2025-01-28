#version 330 core
in vec2 fragUV;
flat in vec4 fragColor;
flat in vec3 fogCol;
in float fogDen;

out vec4 outColor;

uniform sampler2D tex;

void main() {
    vec4 color = texture(tex, fragUV);
    if (color.a < 1e-7) discard;
    outColor = mix(color * fragColor, vec4(fogCol, 1), fogDen);
}