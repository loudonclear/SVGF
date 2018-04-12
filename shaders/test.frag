#version 400

in vec2 uv;

layout(location = 0) out vec4 fragColor;

uniform sampler2D gDepthIds;
uniform sampler2D gNormal;

void main() {

    float clipDepth = texture(gDepthIds, uv).r;

    vec3 normal = texture(gNormal, uv).rgb;

    fragColor = vec4(normal, 1.0);
}
