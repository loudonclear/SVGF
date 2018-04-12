#version 400

layout (location = 0) out vec3 gDepthIds;
layout (location = 1) out vec3 gNormal;

out vec4 fragColor;

in float clipDepth;
in vec3 normal;

uniform int meshID = -1;
uniform int matID = -1;

void main()
{
    gDepthIds = vec3(clipDepth, meshID, matID);
    gNormal = normalize(normal);
}
