#version 400

in vec2 uv;

out vec4 cv;

uniform sampler2D color;

void main() {
    cv = vec4(texture(color, vec2(uv.x, 1.0 - uv.y)).rgb, 0.0);
}
