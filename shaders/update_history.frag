#version 400

in vec2 uv;

layout (location = 0) out vec4 col_out;

uniform sampler2D new_col; // filtered, accumulated color
uniform sampler2D col_history; // to get history length from 4th channel

void main() {
  vec3 c = texture(new_col, uv).rgb;
  vec3 c_old = texture(col_history, uv).rgb;
  float l = texture(col_history, uv).a;

  // col_out = vec4(1 - (c.r-c_old.r), 1 - (c.g-c_old.g), 1 - (c.b-c_old.b), l);
  col_out = vec4(c.r, c.g, c.b, l);
  // col_out = c;
}
