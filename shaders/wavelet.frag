#version 400

in vec2 uv;

layout(location = 0) out vec4 cvnext;

uniform sampler2D colorVariance;
uniform sampler2D gPositionMeshID;
uniform sampler2D gNormal;

uniform int level = 0;
uniform int support = 2;
uniform float sigmaP = 1.0;
uniform float sigmaN = 128.0;
uniform float sigmaL = 4.0;


const float epsilon = 0.00001;
const float h[25] = float[25](1.0/256.0, 1.0/64.0, 3.0/128.0, 1.0/64.0, 1.0/256.0,
                              1.0/64.0,  1.0/16.0, 3.0/32.0,  1.0/16.0, 1.0/64.0,
                              3.0/128.0, 3.0/32.0, 9.0/64.0,  3.0/32.0, 3.0/128.0,
                              1.0/64.0,  1.0/16.0, 3.0/32.0,  1.0/16.0, 1.0/64.0,
                              1.0/256.0, 1.0/64.0, 3.0/128.0, 1.0/64.0, 1.0/256.0);
const float gaussKernel[9] = float[9](1.0/16.0, 1.0/8.0, 1.0/16.0, 1.0/8.0, 1.0/4.0, 1.0/8.0, 1.0/16.0, 1.0/8.0, 1.0/16.0);

float luma(vec3 c){
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

bool eps_equal(float a, float b){
  return round(a) == round(b);
}

void main() {

    vec3 pPosition = texture(gPositionMeshID, uv).rgb;
    float pMeshID = texture(gPositionMeshID, uv).a;

    vec3 pNormal = texture(gNormal, uv).rgb;
    vec3 pColor = texture(colorVariance, uv).rgb;
    float pLuminance = luma(pColor);


    vec2 texelSize = 1.0 / textureSize(gNormal, 0).xy;
    int step = 1 << level;

    vec3 c = vec3(0.0);
    float v = 0.0;
    float weights = 0.0;

    for (int offsetx = -support; offsetx <= support; offsetx++) {
        for (int offsety = -support; offsety <= support; offsety++) {
            vec2 loc = uv + vec2(step * offsetx * texelSize.x, step * offsety * texelSize.y);
            float qMeshID = texture(gPositionMeshID, loc).a;

            if (eps_equal(pMeshID, qMeshID)) {
                vec3 qPosition = texture(gPositionMeshID, loc).rgb;
                vec3 qNormal = texture(gNormal, loc).rgb;

                vec3 qColor = texture(colorVariance, loc).rgb;
                float qVariance = texture(colorVariance, loc).a;
                float qLuminance = luma(qColor);

                vec3 t = pPosition - qPosition;
                float dist2 = dot(t, t) + t.z * t.z;
                float wp = min(exp(-(dist2)/sigmaP), 1.0);

                float wn = pow(max(0.0, dot(pNormal, qNormal)), sigmaN);

                float gvl = 0.0;
                for (int y0 = -1; y0 <= 1; y0++) {
                    for (int x0 = -1; x0 <= 1; x0++) {
                        gvl += gaussKernel[x0 + 3*y0 + 4] * texture(colorVariance, loc + vec2(x0, y0) * texelSize).a;
                    }
                }
                float wl = min(1.0, exp(-abs(pLuminance - qLuminance) / (sigmaL * sqrt(gvl) + epsilon)));

                float w = wp * wn * wl;
                float weight = h[5*(offsety + support) + offsetx + support] * w;

                c += weight * qColor;
                v += weight * weight * qVariance;
                weights += weight;
            }
        }
    }

    if (weights > epsilon) {
        cvnext.rgb = c / weights;
        cvnext.a = v / (weights * weights);
    } else {
        cvnext = texture(colorVariance, uv).rgba;
    }
    cvnext.rgb = clamp(cvnext.rgb, vec3(0.0), vec3(100.0));
}
