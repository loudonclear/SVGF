#version 400

in vec2 uv;

out vec4 fragColor;

uniform sampler2D direct;
uniform sampler2D indirect;
uniform sampler2D albedo;
uniform sampler2D direct1spp;
uniform sampler2D indirect1spp;

uniform int mode;

void main() {
    vec3 directColor = texture(direct, uv).rgb;
    vec3 indirectColor = texture(indirect, uv).rgb;
    vec2 texelSize = 1.0 / textureSize(albedo, 0).xy;
    vec3 albedoColor = texture(albedo, uv).rgb;

    vec3 color;
    if (mode == 0) {
        color = (directColor + indirectColor) * albedoColor;
    } else if (mode == 1) {
        color = directColor;
    } else if (mode == 2){
        color = indirectColor;
    } else {
        if (uv.x > 0.5) {
            directColor = texture(direct1spp, uv).rgb;
            indirectColor = texture(indirect1spp, uv).rgb;
        }
        color = (directColor + indirectColor) * albedoColor;
    }

    fragColor = vec4(color.r / (1 + color.r), color.g / (1 + color.g), color.b / (1 + color.b), 1.0);
}
