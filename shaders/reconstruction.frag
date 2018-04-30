#version 400

in vec2 uv;

out vec4 fragColor;

uniform sampler2D direct;
uniform sampler2D indirect;
uniform sampler2D albedo;

void main() {
    vec3 directColor = texture(direct, uv).rgb;
    vec3 indirectColor = texture(indirect, uv).rgb;
    vec3 albedoColor = texture(albedo, uv).rgb;

    vec3 combined;
    combined = directColor;

    fragColor = vec4(combined.r / (1 + combined.r), combined.g / (1 + combined.g), combined.b / (1 + combined.b), 1.0);
}
