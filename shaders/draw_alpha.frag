#version 400

in vec2 uv;

out vec4 cv;

uniform sampler2D color;

void main() {
  float alpha = round(texture(color, uv).a);
  if (isnan(alpha)){
      cv = vec4(0);
    }
  else if(alpha == 0){
      cv = vec4(0);
    }
    else if(alpha == 1){
      cv = vec4(1,0,0,0);
    }
    else if(alpha == 2){
      cv = vec4(0,1,0,0);
    }
    else if(alpha == 3){
      cv = vec4(0,0,1,0);
    }
    else  if (alpha > 3){
      cv = vec4(1,1,0,0);
    } else {
      cv = vec4(0,1,1,0);
    }
  // cv = vec4(alpha, alpha, alpha, alpha);
}
