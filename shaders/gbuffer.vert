#version 400

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

out vec3 position;
out vec3 normal;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

void main()
{
    vec4 worldPos = m * vec4(aPosition, 1.0);
    position = worldPos.xyz;

    normal = transpose(inverse(mat3(m))) * aNormal;

    gl_Position = p * v * worldPos;
}
