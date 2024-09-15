#include <GLFW/glfw3.h>
#include <glad/glad.h>
// #include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "EBO.h"
#include "VAO.h"
#include "VBO.h"
#include "camera.h"
#include "cube.h"
#include "shaderClass.h"
#include "wave.h"

/*
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#include <cufft.h>

#include "cudaWave.h"
*/

using namespace std;
using namespace glm;

const unsigned int width = 800;
const unsigned int height = 600;
double preX = -1.0;
double preY = -1.0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float currentFrame = 0.0f;

Camera camera(width / height);

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, 0.1f);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, 0.1f);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, 0.1f);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, 0.1f);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    camera.ProcessKeyboard(UP, 0.1f);
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    camera.ProcessKeyboard(DOWN, 0.1f);
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    cout << "Mouse button callback" << endl;
    cout << "xpos: " << xpos << " ypos: " << ypos << endl;
  }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (preX < 0.0f) {
    preX = xpos;
    preY = ypos;
  } else {
    camera.ProcessMouseMovement(float(xpos - preX), float(preY - ypos));
    preX = xpos;
    preY = ypos;
  }
}

void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  cout << "UPDATING ZOOM" << endl;
  camera.ProcessMouseScroll(float(yoffset));
}

int main() {
  // Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(width, height, "APP", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(
      window);   // Introduce the window into the current context
  gladLoadGL();  // Load GLAD so it configures OpenGL
  /*
  glfwSetInputMode(window, GLFW_CURSOR,
                   GLFW_CURSOR_DISABLED);  // Hide the cursor
  */
  // Specify the viewport of OpenGL in the Window
  // In this case the viewport goes from x = 0, y = 0, to w, h
  glViewport(0, 0, width, height);

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // set the clear color
  glClearColor(0.529f, 0.927f, 0.980f, 1.0f);

  Shader oceanShader("ocean.vs", "ocean.fs");
  Shader cubeShader("default.vs", "default.fs");

  // CudaWave wave = CudaWave();
  Wave wave = Wave();
  wave.setCamera(&camera);
  wave.setShader(&oceanShader);
  wave.generatePhillipsSpectrum();

  Cube cube(&cubeShader);

  //   Setup callback functions
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, mouse_scroll_callback);

  // Game loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    wave.update();
    cube.render(&camera, vec3(0.0f, 10.0f, 10.0f), vec3(1.0f, 1.0f, 1.0f),
                vec3(1.0f, 0.0f, 0.0f));
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
