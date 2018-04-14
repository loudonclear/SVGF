#version 400

in vec2 uv;

layout(location = 0) out vec4 fragColor;

uniform sampler2D directTexture;

void main() {
    vec2 uvu = vec2(uv.x, 1.0 - uv.y);
    fragColor = vec4(texture(directTexture, uvu).rgb, 1.0);
}
