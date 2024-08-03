#version 410 core

in vec2 fuv;
in vec2 cuv;
flat in float tex_t;
flat in vec4 fragColor;

layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;

uniform sampler2D tex;

void main() {
    vec4 color = mix(texture(tex, fuv), texture(tex, cuv), tex_t) * fragColor;

    // weight function
    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

    // store pixel color accumulation
    accum = vec4(color.rgb * color.a, color.a) * weight;

    // store pixel revealage threshold
    reveal = color.a;
}