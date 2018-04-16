#version 420

in vec2 uv;

out vec4 fragColor;

uniform sampler2D colorVariance;

void main() {
    fragColor = vec4(texture(colorVariance, uv).rgb, 1.0);
}
