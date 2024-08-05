[{tex,4}]
#version 410

out vec4 color;
uniform sampler2D tex;

void main() {
    color = vec4(texture(tex, gl_FragCoord.xy / textureSize(tex, 0)).rgb, 1.0);
}