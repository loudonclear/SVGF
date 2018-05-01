#version 400

in vec2 uv;

out vec4 fragColor;

uniform sampler2D direct;
uniform sampler2D indirect;
uniform sampler2D albedo;

void main() {
    const vec2 offset = vec2(0, 0);
    vec3 directColor = texture(direct, uv).rgb;
    vec3 indirectColor = texture(indirect, uv).rgb;
    vec2 texelSize = 1.0 / textureSize(albedo, 0).xy;
    vec3 albedoColor = texture(albedo, uv).rgb;

    vec3 combined;
    combined = directColor;
    combined = (directColor + indirectColor) * albedoColor;

    fragColor = vec4(combined.r / (1 + combined.r), combined.g / (1 + combined.g), combined.b / (1 + combined.b), 1.0);
}
