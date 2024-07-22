#version 330

in vec2 fragUV;
out vec4 color;

void main() {
    if (abs(fragUV.x) < 0.97 && abs(fragUV.y) < 0.97)
        discard;
    color = vec4(0.0, 0.0, 0.0, 1.0);
}