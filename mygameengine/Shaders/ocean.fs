#version 330 core

in vec3 FragPos;    // World-space position of the fragment
in vec2 TexCoord;   // Texture coordinates for sampling
in vec3 Normal;     // Surface normal for lighting

out vec4 FragColor;

uniform sampler2D heightMap;  // The height field texture

uniform vec3 lightPosition;   // Position of the light source
uniform vec3 lightColor;      // Light color
uniform vec3 waterColorDeep;  // Deep water color
uniform vec3 waterColorShallow; // Shallow water color

void main() {
    // Get height value from the height map texture
    float height = texture(heightMap, TexCoord).r;

    // Map height to a color: interpolate between deep blue and shallow blue based on the height
    vec3 waterColor = mix(waterColorDeep, waterColorShallow, height);

    // Lighting calculation (basic diffuse lighting)
    vec3 lightDir = normalize(lightPosition - FragPos); // Direction from fragment to light
    vec3 norm = normalize(Normal);                     // Normal at the fragment
    float diff = max(dot(norm, lightDir), 0.0);        // Diffuse intensity

    // Apply diffuse lighting to the water color
    vec3 resultColor = waterColor * diff * lightColor;

    // Output final fragment color
    FragColor = vec4(resultColor, 1.0);
}
