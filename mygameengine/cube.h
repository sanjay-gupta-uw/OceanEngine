#ifndef CUBE_H
#define CUBE_H

#include <glm/glm.hpp>
#include <vector>
#include "shaderClass.h"
#include "camera.h"

class Cube
{
public:
    // Constructor and destructor
    Cube(Shader *shader);
    Cube() {};
    ~Cube();

    // Methods to initialize and render the cube
    void init();
    void render(Camera *camera, glm::vec3 lightPos, glm::vec3 lightColor, glm::vec3 cubeColor);

private:
    GLuint VAO, VBO, EBO;
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    Shader *shader;

    void setupCube();
};

#endif
