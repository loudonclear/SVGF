#version 400

layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;

void main() {
    // TODO: Transform the position using matrix uniforms. (Task 2)
    gl_Position = vec4(position, 1.0);
}
