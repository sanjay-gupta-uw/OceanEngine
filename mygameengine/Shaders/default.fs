#version 330 core

in vec3 FragPos;    // Fragment position in world space (from vertex shader)
in vec3 Normal;     // Normal vector (from vertex shader)

out vec4 FragColor; // Output color

uniform vec3 lightPosition;  // Position of the light in world space
uniform vec3 lightColor;     // Color of the light
uniform vec3 objectColor;    // Color of the cube

void main() {
    // Ambient lighting
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Combine ambient and diffuse lighting
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}
