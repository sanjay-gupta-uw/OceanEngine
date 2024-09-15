/*
#pragma once
#ifndef CUDA_WAVE_CLASS_H
#define CUDA_WAVE_CLASS_H

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#include <cufft.h>
#include <glad/glad.h>

#include <complex>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>

#include "camera.h"
#include "shaderClass.h"
#include "waveKernels.cuh"

#define M_PI 3.14159265358979323846f
#define M_GRAVITY 9.81f

class CudaWave {
 private:
  GLuint cube_vbo, cube_vao;
  GLuint pos_vbo, vao, ibo;
  Shader* shader;

  cufftHandle plan;
  float2* cuda_h0;
  float2* cuda_ht;
  float2* slope_ptr;
  float* height_ptr;

  GLuint height_vbo, slope_vbo;

  struct cudaGraphicsResource *cuda_heightVB_resource,
      *cuda_slopeVB_resource;  // to help CUDA write to the VBO buffers

  void initCuda();

  void createMesh(int width, int height);
  void createMeshIndexBuffer(int w, int h);
  void compileShaders();

  void renderMesh(Camera* camera);

  void generateInitialSpectrum(float2* h0);

 public:
  CudaWave();
  ~CudaWave();
  void update(float t, Camera* camera);
};

#endif
*/