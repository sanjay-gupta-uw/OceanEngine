/*
#ifndef WAVE_KERNELS_H
#define WAVE_KERNELS_H

#include <cuda_runtime.h>
#include <cufft.h>
#include <math_constants.h>

#include "device_launch_parameters.h"

#define CUDART_GRAVITY_F 9.81f
// Kernel wrapper function declarations
extern "C" void cudaGenerateTimeSpectrumKernel(float2* d_h0, float2* d_ht,
                                               unsigned int in_width,
                                               unsigned int out_width,
                                               unsigned int out_height,
                                               float TIME, float L);

extern "C" void cudaUpdateHeightmapKernel(float* d_heightMap, float2* d_ht,
                                          unsigned int width,
                                          unsigned int height);

extern "C" void cudaCalculateSlopeKernel(float* hptr, float2* slopeOut,
                                         unsigned int width,
                                         unsigned int height);

#endif

*/