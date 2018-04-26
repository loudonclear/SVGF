#version 400

layout (location = 0) out vec4 gPositionMeshID;
layout (location = 1) out vec3 gNormal;

in vec3 position;
in vec3 normal;

uniform float meshID;
uniform float matID;

void main()
{
    gPositionMeshID = vec4(position, meshID);
    gNormal = normalize(normal);
}
