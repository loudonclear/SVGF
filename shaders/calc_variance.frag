#version 400

// Shader for taking color & moments history and outputting color and estimated variance.

// TODO Spatial variance estimation

in vec2 uv;

layout(location = 0) out vec4 fragColorVariance;

uniform sampler2D col_history;
uniform sampler2D moments_history;

void main() {
  vec3 col = texture(col_history, uv).rgb;
  float  history_length = texture(col_history, uv).a;
  // TODO if history_length < 4, estimate variance spatially
  vec2 raw_moments = texture(moments_history, uv).rg;
  float mu_1 = raw_moments.x;
  float mu_2 = raw_moments.y;
  // variance = E[X^2] - E[X]^2
  float variance = mu_2 - mu_1 * mu_1;
  fragColorVariance = vec4(col, abs(variance));
}
