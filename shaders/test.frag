#version 400

in vec2 uv;

out vec4 fragColor;

uniform sampler2D color;

void main() {
    fragColor = vec4(texture(color, uv).rgb, 1.0);
}
