#version 400

in vec2 uv;

layout (location = 0) out vec3 uv_prev; // u, v, and consistency

// Previous frame camera
uniform mat4 v_prev;
uniform mat4 p_prev;

uniform sampler2D pos_id_prev;
uniform sampler2D normals_prev;
uniform sampler2D pos_id;
uniform sampler2D normals;

#define M_PI 3.1415926535897932384626433832795

// world_pos = m * aPosition (whatever aPosition is)
vec4 project(vec3 world_pos, mat4 v, mat4 p) {
  vec4 world_pos4 = vec4(world_pos, 1.0);
  return p * v * world_pos4;
}

vec4 project_reverse(vec3 screen_pos, mat4 v, mat4 p){
  vec4 screen_pos4 = vec4(screen_pos, 1.0);
  return inverse(v) * inverse(p) * screen_pos4;

}

void main() {
  float id = texture(pos_id, uv).a;
  vec3 n = texture(normals, uv).xyz;

  // backproject current pos to world space
  vec4 world_pos = vec4(texture(pos_id, uv).xyz, 1.0);

  // project world space to previous pos
  vec4 point_prev = project(world_pos.xyz, v_prev, p_prev);
  uv_prev.xy = 0.5 * (point_prev.xy / point_prev.w + vec2(1,1));
  // uv_prev.xy = point_prev.xy / point_prev.z;

  // consistency check current and previous pos
  // float id_prev = texture(pos_id_prev, abs(uv_prev.xy)).a;
  float id_prev = texture(pos_id_prev, abs(uv_prev.xy)).a;
  vec3 n_prev = texture(normals_prev, abs(uv_prev.xy)).xyz;
  // TODO what to do in case of background
  bool consistency = (id_prev == id) && (dot(n, n_prev) > M_PI / 4);
  uv_prev.z = float(consistency);
  // uv_prev.xyz = vec3(length(uv.xy - uv_prev.xy) * 200);
}
