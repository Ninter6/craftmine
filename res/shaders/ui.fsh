[{tex,7}]
#version 410 core

in vec2 fragUV;
out vec4 frag;

uniform sampler2D tex;

void main() {
    frag = vec4(texture(tex, fragUV).rgb, 1.0);
}