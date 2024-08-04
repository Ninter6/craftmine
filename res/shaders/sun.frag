[{tex,3}]
#version 330

in vec3 fragPos;
in vec2 fragUV;
out vec4 color;

uniform sampler2D tex;

void main() {
    color = texture(tex, fragUV);
    if (color.a < 1e-7 || fragPos.y < 0.03) discard;
}