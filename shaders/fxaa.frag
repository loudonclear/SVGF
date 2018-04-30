#version 400

in vec2 uv;

out vec4 fragColor;

uniform sampler2D color;

uniform float lumaThreshold = 0.85;
uniform float mulReduce = 1.0 / 8.0;
uniform float minReduce = 1.0 / 128.0;
uniform float maxSpan = 8.0;


float luma(vec3 c){
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

void main() {
    vec2 texelSize = 1.0 / textureSize(color, 0).xy;
    vec3 c = texture(color, uv).rgb;
    vec3 ul = textureOffset(color, uv, ivec2(-1, 1)).rgb;
    vec3 ur = textureOffset(color, uv, ivec2(1, 1)).rgb;
    vec3 dl = textureOffset(color, uv, ivec2(-1, -1)).rgb;
    vec3 dr = textureOffset(color, uv, ivec2(1, -1)).rgb;

    float lumac = luma(c);
    float lumaul = luma(ul);
    float lumaur = luma(ur);
    float lumadl = luma(dl);
    float lumadr = luma(dr);

    float lumaMin = min(lumac, min(min(lumaul, lumaur), min(lumadl, lumadr)));
    float lumaMax = max(lumac, max(max(lumaul, lumaur), max(lumadl, lumadr)));

    if (lumaMax - lumaMin < lumaMax * lumaThreshold) {
        fragColor = vec4(c, 1.0);
        return;
    }

    // Gradient.
    vec2 samplingDirection = vec2((lumadl + lumadr) - (lumaul + lumaur), (lumaul + lumadl) - (lumaur + lumadr));
    float samplingDirectionReduce = max((lumaul + lumaur + lumadl + lumadr) * 0.25 * mulReduce, minReduce);
    float minSamplingDirectionFactor = 1.0 / (min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);
    samplingDirection = clamp(samplingDirection * minSamplingDirectionFactor, vec2(-maxSpan, -maxSpan), vec2(maxSpan, maxSpan)) * texelSize;


    vec3 innerNeg = texture(color, uv + samplingDirection * (1.0/3.0 - 0.5)).rgb;
    vec3 innerPos = texture(color, uv + samplingDirection * (2.0/3.0 - 0.5)).rgb;
    vec3 outerNeg = texture(color, uv + samplingDirection * (0.0/3.0 - 0.5)).rgb;
    vec3 outerPos = texture(color, uv + samplingDirection * (3.0/3.0 - 0.5)).rgb;

    vec3 halfAverage = (innerPos + innerNeg) * 0.5;
    vec3 fullAverage = (outerPos + outerNeg) * 0.25 + halfAverage * 0.5;
    float lumafull = luma(fullAverage);

    if (lumafull < lumaMin || lumafull > lumaMax) {
        fragColor = vec4(halfAverage, 1.0);
    }
    else {
        fragColor = vec4(fullAverage, 1.0);
    }
}
