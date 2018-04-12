#version 400

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;

//in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform float meshID;
uniform float matID;
//uniform sampler2D texture_diffuse;

void main()
{
    gPosition = vec4(FragPos, meshID);
    gNormal = vec4(normalize(Normal), matID);
}
