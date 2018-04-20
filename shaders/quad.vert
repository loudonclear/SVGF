#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inUV;

out vec2 uv;

void main() {
    uv = inUV;
    gl_Position = vec4(position, 1.0);
}
