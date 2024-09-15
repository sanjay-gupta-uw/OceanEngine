#version 330 core

layout(location = 0) in vec3 aPosition;  // The vertex position (X, Y, Z)
layout(location = 1) in vec2 aTexCoord;  // The texture coordinates (for heightfield lookup)

uniform sampler2D heightMap;  // The height field texture
uniform float heightScale;    // Scale factor to control the height displacement

uniform mat4 view;            // Camera view matrix
uniform mat4 projection;      // Camera projection matrix
uniform mat4 model;           // Model matrix for ocean transformation

out vec3 FragPos;    // World-space position of the fragment
out vec2 TexCoord;   // Texture coordinates for sampling
out vec3 Normal;     // Surface normal for lighting calculations

void main() {
    // Get the height value from the texture using the texture coordinates
    float height = texture(heightMap, aTexCoord).r;

    // Displace the vertex's Y position based on the height and a scale factor
    vec3 displacedPosition = aPosition;
    displacedPosition.y += height * heightScale;

    // Compute the final world-space position of the vertex
    FragPos = vec3(model * vec4(displacedPosition, 1.0));

    // Calculate approximate normals (assumes flat grid, for better results you'd want to calculate normals in a more advanced way)
    Normal = normalize(vec3(0.0, 1.0, 0.0));  // Assuming normals point upwards, modify if using actual normals

    // Pass the displaced position to the next stage
    gl_Position = projection * view * vec4(FragPos, 1.0);

    // Pass the texture coordinates to the fragment shader
    TexCoord = aTexCoord;
}
