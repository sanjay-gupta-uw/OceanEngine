#include "Cube.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
using namespace std;

Cube::Cube(Shader *shader) : shader(shader) { init(); }

Cube::~Cube()
{
    // Cleanup VAO, VBO, and EBO
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    delete shader;
}

void Cube::init()
{
    // Define cube vertices and indices
    vertices = {// Positions (X, Y, Z)
                glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
                glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
                glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
                glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f)};

    // Define the cube's indices for indexed drawing
    indices = {
        0, 1, 2, 2, 3, 0, // Front face
        4, 5, 6, 6, 7, 4, // Back face
        4, 5, 1, 1, 0, 4, // Bottom face
        7, 6, 2, 2, 3, 7, // Top face
        4, 7, 3, 3, 0, 4, // Left face
        5, 6, 2, 2, 1, 5  // Right face
    };

    setupCube(); // Setup VAO, VBO, EBO
}

void Cube::setupCube()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Bind vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                 &vertices[0], GL_STATIC_DRAW);

    // Bind element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);
}

void Cube::render(Camera *camera, glm::vec3 lightPos, glm::vec3 lightColor,
                  glm::vec3 cubeColor)
{
    // Use the cube shader program
    shader->Bind();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix();

    // Set the model matrix (identity matrix for static cube)
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE,
                       &model[0][0]);
    // Set the view and projection matrices in the shader
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                       &view[0][0]);
    // Set the projection matrix in the shader
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                       GL_FALSE, &projection[0][0]);

    // Set lighting and material properties in fragment shader
    glUniform3fv(glGetUniformLocation(shader->ID, "lightPosition"), 1,
                 &lightPos[0]);
    glUniform3fv(glGetUniformLocation(shader->ID, "lightColor"), 1,
                 &lightColor[0]);
    glUniform3fv(glGetUniformLocation(shader->ID, "objectColor"), 1,
                 &cubeColor[0]);

    // Bind the cube VAO and draw it
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}
