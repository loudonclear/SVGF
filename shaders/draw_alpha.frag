#version 400

in vec2 uv;

out vec4 cv;

uniform sampler2D color;

void main() {
    float alpha = texture(color, uv).a;
    cv = vec4(alpha, alpha, alpha, alpha);
}
