#version 400

layout(location = 0) in vec3 position; // Position of the vertex
layout(location = 1) in vec3 normal;   // Normal of the vertex
//layout(location = 5) in vec2 texCoord; // UV texture coordinates

out vec3 color; // Computed color for this vertex

// Transformation matrices
uniform mat4 p;
uniform mat4 v;
uniform mat4 m;

// Material data
uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float shininess;

// Light data
uniform vec3 light_dir;
uniform vec3 light_color;

void main() {
    vec4 position_cameraSpace = v * m * vec4(position, 1.0);
    vec4 normal_cameraSpace = vec4(normalize(mat3(transpose(inverse(v * m))) * normal), 0);

    vec4 position_worldSpace = m * vec4(position, 1.0);
    vec4 normal_worldSpace = vec4(normalize(mat3(transpose(inverse(m))) * normal), 0);

    gl_Position = p * position_cameraSpace;


    // Add ambient component
    color = ambient_color.xyz;

    vec4 vertexToLight = normalize(v * vec4(-light_dir, 0));

    // Add diffuse component
    float diffuseIntensity = max(0.0, dot(vertexToLight, normal_cameraSpace));
    color += max(vec3(0), light_color * diffuse_color * diffuseIntensity);

    // Add specular component
    vec4 lightReflection = normalize(-reflect(vertexToLight, normal_cameraSpace));
    vec4 eyeDirection = normalize(vec4(0,0,0,1) - position_cameraSpace);
    float specIntensity = pow(max(0.0, dot(eyeDirection, lightReflection)), shininess);
    color += max (vec3(0), light_color * specular_color * specIntensity);

    color = clamp(color, 0.0, 1.0);
}
