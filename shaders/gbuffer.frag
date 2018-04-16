#version 420

layout (location = 0) out vec3 gDepthIDs;
layout (location = 1) out vec3 gNormal;

in float clipDepth;
in vec3 normal;

uniform float meshID;
uniform float matID;

void main()
{
    gDepthIDs = vec3(clipDepth, meshID, matID);
    gNormal = normalize(normal);
}
