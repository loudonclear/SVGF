#version 400

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

out float clipDepth;
out vec3 normal;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

void main()
{
    gl_Position = p * v * m * vec4(aPosition, 1.0);
    clipDepth = gl_Position.z;

    normal = transpose(inverse(mat3(m))) * aNormal;
}
