#ifndef WAVE_H
#define WAVE_H

#include <fftw3.h>

#include <cmath>
#include <complex>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include <random>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "shaderClass.h"

class Wave
{
  // wave parameters
  std::complex<float> *h0_k_;
  std::complex<float> *h_kt_;

  float A;
  float g;
  float v;

  float currentFrame = 0.0f;
  float lastFrame = 0.0f;
  float deltaTime = 0.0f;
  float timeStep = 0.0f; // 't' value used for ocean simulation

  Camera *camera;
  Shader *shader;

  glm::vec3 *vertices;
  std::vector<unsigned int> indices;
  glm::vec2 *texCoords;
  GLuint VAO, VBO, EBO, texVBO;
  GLuint heightMapTexture;

  // wave functions

public:
  Wave();
  ~Wave();

  void setCamera(Camera *camera);
  void setShader(Shader *shader);

  void initRenderParams();

  float Phillips(glm::vec2 K);
  void generatePhillipsSpectrum();
  void generateH_KT_Spectrum(float t);
  void generateHeightField();
  void saveAsImage(float brightnessScale, int option = 0);
  void saveHeightFieldAsImage(fftwf_complex *heightField);
  void createSurface();
  void update();
  void render();
};

#endif