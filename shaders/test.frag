#version 400

in vec2 uv;

out vec4 fragColor;

uniform isampler2D gMeshMatID;
uniform sampler2D gNormal;

void main() {
    int meshID = texture(gMeshMatID, uv).r;
    int matID = texture(gMeshMatID, uv).g;

    vec3 normal = texture(gNormal, uv).rgb;

    if (meshID == 0) {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }

}
