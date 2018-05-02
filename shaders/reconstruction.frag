#version 400

in vec2 uv;

out vec4 fragColor;

uniform sampler2D direct;
uniform sampler2D indirect;
uniform sampler2D albedo;

uniform int mode;

void main() {
    const vec2 offset = vec2(0, 0);
    vec3 directColor = texture(direct, uv).rgb;
    vec3 indirectColor = texture(indirect, uv).rgb;
    vec2 texelSize = 1.0 / textureSize(albedo, 0).xy;
    vec3 albedoColor = texture(albedo, uv).rgb;

    vec3 color;
    if (mode == 0) {
        color = (directColor + indirectColor) * albedoColor;
    } else if (mode == 1) {
        color = directColor;
    } else {
        color = indirectColor;
    }

    fragColor = vec4(color.r / (1 + color.r), color.g / (1 + color.g), color.b / (1 + color.b), 1.0);
}
