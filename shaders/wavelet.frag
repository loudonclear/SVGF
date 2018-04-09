#version 400 core

in vec2 uv;

out vec4 cvnext;

uniform sampler2D gPosition; // TODO: Add mesh id as 4th component
uniform sampler2D gNormal;
uniform sampler2D colorVariance;

uniform int level;

const int support = 2;
const float epsilon = 0.00001;
const float sigmaZ = 1.0;
const float sigmaN = 128.0;
const float sigmaL = 4.0;
const float h[5] = float[5](1.0/16.0, 1.0/4.0, 3.0/8.0, 1.0/4.0, 1.0/16.0);


float luma(vec3 rgb) {
    return (rgb.r + rgb.r + rgb.b + rgb.g + rgb.g + rgb.g) / 6.0;
}

// TODO: standard luminance formula function


// REMEMBER: Alternate horizontal and vertical. Then up the level. Repeat. After first iter, store colorVariance in separate texture.

// Horizontal wavelet (might be able to change texcoords for vertical pass)
void main() {
    vec3 fragpos = texture(gPosition, uv).rgb;
    int meshID = int(texture(gPosition, uv).w);
    vec3 normal = texture(gNormal, uv).rgb;
    vec3 color = texture(colorVariance, uv).rgb;
    float variance = texture(colorVariance, uv).w;
    float luminance = 0.0;

    // TODO: Might need normalization
    float depth = fragpos.z;

    vec2 texelSize = 1.0 / textureSize(gPosition, 0).xy;
    int step = 1 << level;

    vec3 c = vec3(0.0, 0.0, 0.0);
    float v = 0.0;
    float weights = 0.0;

    for (int xoffset = -support; xoffset <= support; xoffset++) {
        vec2 loc = uv + vec2(step * xoffset * texelSize.x);
        int m = int(texture(gPosition, loc).a);

        if (meshID == m) {
            vec3 p = texture(gPosition, loc).rgb;
            vec3 n = texture(gNormal, loc).rgb;
            float d = p.z;

            float dz = depth - d;
            float wz = min(1.0, exp(-abs(dz) / (sigmaZ * abs(dz * (uv.x - loc.x)) + epsilon)));

            float wn = pow(max(0, dot(normal, n)), sigmaN);

            // TODO: luminance
            float l = 0.0;
            float gvl = 0.0;
            float wl = min(1.0, exp(-abs(luminance - l)) / (sigmaL * gvl + epsilon));

            float w = wz * wn * wl;
            float weight = h[xoffset + support] * w;

            c += weight * color;
            v += weight * weight * variance;
            weights += weight;
        }
    }

    if (weights > 0.0) {
        cvnext.rgb = c / weights;
        cvnext.a = v / (weights * weights);
    }
}
