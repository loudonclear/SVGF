#version 400

layout (location = 0) out ivec2 gMeshMatID;
layout (location = 1) out vec3 gNormal;

in float clipDepth;
in vec3 normal;

uniform int meshID = -1;
uniform int matID = -1;

void main()
{
    gMeshMatID = ivec2(meshID, matID);
    gNormal = normalize(normal);
}
