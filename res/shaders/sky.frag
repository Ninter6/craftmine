#version 330 core

in vec3 fragDir;

flat in vec3 camDir;
flat in vec3 camPos;

flat in vec3 sunDir;

out vec4 color;

uniform sampler2D transmittance;

const float sea_level = 32;
const float radius = 512;

vec3 RayleighCoefficient(float h) {
    const vec3 sigma = vec3(5.802, 13.558, 33.1) * 1e-6;
    const float H = 8500;
    return sigma * exp(-h / H);
}
float RayleiPhase(float cs) {
    return 0.0596831 * (1 + cs * cs);
}

vec3 MieCoefficient(float h) {
    const vec3 sigma = vec3(3.996 * 1e-6).xxx;
    const float H = 1200;
    return sigma * exp(-h / H);
}

float MiePhase(float cs) {
    const float g = 0.2;

    float a = 0.1193662;
    float b = (1 - g*g) / (2 + g*g);
    float c = 1 + cs*cs;
    float d = pow(1 + g*g - 2*g*cs, -1.5);

    return a*b*c*d;
}

vec3 Scattering(vec3 p, vec3 inDir, vec3 outDir) {
    float cs = dot(inDir, outDir);
    float h = length(p) - sea_level;
    vec3 rayleigh = RayleighCoefficient(h) * RayleiPhase(cs);
    vec3 mie = MieCoefficient(h) * MiePhase(cs);

    return rayleigh + mie;
}

vec3 MieAbsorption(float h) {
    const vec3 sigma = vec3(4.4 * 1e-6).xxx;
    const float H = 1200;
    return sigma * exp(-h / H);
}
vec3 OzenoAbsorption(float h) {
    const vec3 sigma = vec3(0.65, 1.881, 0.085) * 1e-6;
    float center = 25;
    float width = 15;
    float rho = max(0, 1.0 - abs(h - center) / width);
    return sigma * rho;
}

vec2 GetTransmittanceUV(float br, float tr, float mu, float r) {
    float H = sqrt(tr*tr - br*br);
    float rho = sqrt(r*r - br*br);

    float dis = r*r * (mu*mu - 1.0) + tr*tr;
    float d = max(0, (-r * mu +sqrt(dis)));

    float d_min = tr - r;
    float d_max = rho + H;

    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;

    return vec2(x_mu, x_r);
}

vec3 Transmittance(vec3 p, vec3 dir) {
    float br = sea_level;
    float tr = radius;

    vec3 U = normalize(p);
    float cs = dot(dir, U);
    float r = length(p);

    vec2 uv = GetTransmittanceUV(br, tr, cs, r);
    return texture(transmittance, uv).rgb;
}

float RaySphere(float r, vec3 o, vec3 d) {
    float b = 2*dot(o, d);
    float c = dot(o, o) - r*r;
    return (-b + sqrt(b*b - 4*c)) * 0.5;
}

void main() {
    vec3 f_dir = normalize(fragDir);

    vec3 p1 = camPos;
    float len = RaySphere(radius, p1, f_dir);
    float ds = len / 32;
    vec3 D = vec3(0);
    vec3 col = vec3(0);
    for (int i = 0; i < 32; i++) {
        float h = length(p1) - sea_level;
        vec3 E = RayleighCoefficient(h) + MieCoefficient(h) + OzenoAbsorption(h) + MieAbsorption(h);
        D += E * ds;

        vec3 t1 = Transmittance(p1, sunDir);
        vec3 s = Scattering(p1, sunDir, camDir);
        vec3 t2 = exp(-D);

        vec3 inS = t1 * s * t2 * ds * 10;
        col += inS;
        p1 += camDir * ds;
    }

    color = vec4(0.4, 0.8, 1.0, 1.0);
}