#version 420

in vec2 uv;

layout(location = 0) out vec4 cvnext;

uniform sampler2D gDepthIDs;
uniform sampler2D gNormal;
uniform sampler2D colorVariance;
//uniform sampler2D luma;

uniform int level;

const int support = 2;
const float epsilon = 0.00001;
const float sigmaZ = 1.0;
const float sigmaN = 128.0;
const float sigmaL = 4.0;
const float h[5] = float[5](1.0/16.0, 1.0/4.0, 3.0/8.0, 1.0/4.0, 1.0/16.0);
const float gaussKernel[9] = float[9](1.0/16.0, 1.0/8.0, 1.0/16.0, 1.0/8.0, 1.0/4.0, 1.0/8.0, 1.0/16.0, 1.0/8.0, 1.0/16.0);

//// May also need gamma correction
//const vec3 lumaConvert = vec3(0.2126, 0.7152, 0.0722);
//float luma(vec3 rgb) {
//    return dot(lumaConvert, rgb);
//}


// Horizontal wavelet
// REMEMBER: Alternate horizontal and vertical ping-ponging colorVariance. Then up the level. Repeat.

void main() {

    float pDepth = texture(gDepthIDs, uv).r;
    float pMeshID = texture(gDepthIDs, uv).g;
    float pMatID = texture(gDepthIDs, uv).b;

    vec3 pNormal = texture(gNormal, uv).rgb;

    float pLuminance = 0.0;//texture(luma, uv).r;


    vec2 texelSize = 1.0 / textureSize(gDepthIDs, 0).xy;
    int step = 1 << level;

    vec3 c = vec3(0.0);
    float v = 0.0;
    float weights = 0.0;

    for (int offset = -support; offset <= support; offset++) {
        vec2 loc = uv + vec2(0.0, step * offset * texelSize.y);
        float qMeshID = texture(gDepthIDs, loc).g;
        float qMatID = texture(gDepthIDs, loc).b;

        if (pMeshID == qMeshID && pMatID == qMatID) {
            float qDepth = texture(gDepthIDs, loc).r;
            vec3 qNormal = texture(gNormal, loc).rgb;

            vec3 qColor = texture(colorVariance, loc).rgb;
            float qVariance = texture(colorVariance, loc).a;
            float qLuminance = 0.0;//texture(luma, loc).r;

            float dz = pDepth - qDepth;
            float wz = min(1.0, exp(-abs(dz) / (sigmaZ * abs(dz * (uv.x - loc.x)) + epsilon)));

            float wn = pow(max(0.0, dot(pNormal, qNormal)), sigmaN);

            float gvl = 0.0;
            for (int y0 = -1; y0 <= 1; y0++) {
                for (int x0 = -1; x0 <= 1; x0++) {
                    gvl += gaussKernel[x0 + 3*y0 + 4] * texture(colorVariance, loc + vec2(x0, y0) * texelSize).a;
                }
            }
            float wl = min(1.0, exp(-abs(pLuminance - qLuminance)) / (sigmaL * sqrt(gvl) + epsilon));
            wl = 1.0;

            float w = wz * wn * wl;
            float weight = h[offset + support] * w;

            c += weight * qColor;
            v += weight * weight * qVariance;
            weights += weight;
        }
    }

    if (weights > 0.0) {
        cvnext.rgb = c / weights;
        cvnext.a = v / (weights * weights);
    }
}
