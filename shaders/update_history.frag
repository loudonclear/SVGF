#version 400

in vec2 uv;

layout (location = 0) out vec4 cv_out;
layout (location = 1) out vec2 moments_out;

uniform sampler2D history; // to get history length from 4th channel
uniform sampler2D new_moments; // new moments

void main() {
  cv_out = texture(history, uv).rgba;
  moments_out = texture(new_moments, uv).rg;
}
