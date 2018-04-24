#version 400

// Shader for taking history & current frame and create integrated color & variance data to use for filtering.

in vec2 uv;

layout (location = 0) out vec4 fragColorVariance;

uniform float alpha;

uniform sampler2D col_history;
uniform sampler2D current_color;
uniform sampler2D moments;

void main(){
  // TODO right now we're not taking into account motion. Just accumulate color directly;

  // TODO filtered color input based on GBuffer
  vec3 col = texture(current_color, uv).rgb;
  vec3 col_prev = texture(col_history, uv).rgb;
  vec2 moments = texture(moments, uv).rg;
  float l = texture(col_history, uv).a;
  // if l == 0, set alpha to 1 and discard col_prev;
  float alpha_weight = max(float(l == 0), alpha);
  // alpha_weight = alpha; // ignore l for now, just test interpolation
  fragColorVariance = vec4(col * alpha_weight + (1-alpha_weight) * col_prev, l+1);
  // fragColorVariance = vec4(col_prev - col, l);
  // fragColorVariance.r = col_prev.r + col.r + 0.2;
  // fragColorVariance = vec4(col_prev,l);
}
