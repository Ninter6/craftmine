#version 410 core

in vec2 fragUV;
out vec4 frag;

uniform sampler2D tex;

void main() {
    frag = texture(tex, fragUV);
}