#version 330 core

layout(location = 0) in vec3 aPosition;  // Vertex position (X, Y, Z)
layout(location = 1) in vec3 aNormal;    // Vertex normal for lighting

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    // Calculate transformed vertex position and normal for lighting
    FragPos = vec3(model * vec4(aPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  // Adjust normals
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
