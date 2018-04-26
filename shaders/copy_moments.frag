#version 400

// Shader for copying in moment information to history buffer

in vec2 uv;

layout (location = 1) out vec2 out_moments;

uniform sampler2D in_moments;

void main() {
  out_moments = texture(in_moments, uv).rg;
}
