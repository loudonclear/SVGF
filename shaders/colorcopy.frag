#version 400

in vec2 uv;

out vec4 cv;

uniform sampler2D color;

void main() {
    cv = texture(color, uv);
}
