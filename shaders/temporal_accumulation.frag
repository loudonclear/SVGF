#version 400

// Shader for taking history & current frame and create integrated color &
// variance data to use for filtering.

in vec2 uv;

layout(location = 0) out vec4 fragColorVariance;
layout(location = 1) out vec2 out_moments;

uniform float alpha;

uniform sampler2D col_history;
uniform sampler2D motion_vectors;
uniform sampler2D current_color;
uniform sampler2D moments;

float luma(vec3 c){
  return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

void main() {
  // TODO right now we're not taking into account motion. Just accumulate color
  // directly;

  // TODO filtered color input based on GBuffer
  vec3 col = texture(current_color, uv).rgb;
  vec3 prev_uv = texture(motion_vectors, uv).rgb;
  vec3 col_prev = texture(col_history, prev_uv.xy).rgb;
  vec2 moments_prev = texture(moments, uv).rg;
  // if motion vector is invalid, l is 0.
  float l = texture(col_history, uv).a * float(prev_uv.z > 0);

  // if l == 0, set alpha to 1 and discard col_prev;
  float alpha_weight = max(float(l == 0), alpha);
  // alpha_weight = alpha; // ignore l for now, just test interpolation
  if (any(isnan(col_prev))) {
    //  fragColorVariance = vec4(col, l + 1);
    col_prev = vec3(0, 0, 0);
    alpha_weight = 1.0;
  }

  // update data;
  // alpha_weight = alpha;
  fragColorVariance =
      vec4(col * alpha_weight + (1 - alpha_weight) * col_prev, l + 1);
  float new_luma = luma(col);
  vec2 new_moments = vec2(new_luma, new_luma * new_luma);
  out_moments = new_moments * alpha_weight + (1-alpha_weight) * moments_prev;

  if (any(isnan(fragColorVariance))) {
    fragColorVariance = vec4(0,0,0,0);
  }
}
