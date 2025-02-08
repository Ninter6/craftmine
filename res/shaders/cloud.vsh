#version 410 core

layout (location = 0) in vec2 pos;

out vec3 fragPos;
flat out vec3 camPos;
flat out vec4 proj2;
flat out vec4 proj3;

layout(std140) uniform UBO {
    mat4 proj;
    mat4 view;
};

void main() {
    vec2 p = pos * 2 - 1;
    gl_Position = vec4(p, 0, 1);

    camPos = -transpose(mat3(view)) * view[3].xyz;

    mat4 vp = proj * view;
    mat4 inv_vp = inverse(vp);
    vec4 f = vec4(inv_vp * vec4(p, 1, 1));
    fragPos = f.xyz / f.w;

    proj2 = vec4(vp[0][2], vp[1][2], vp[2][2], vp[3][2]);
    proj3 = vec4(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);
}