/*
#include "cudaWave.h"

using namespace std;

const int meshWidth = 256;
const int meshHeight = 256;

const int spectralWidth = meshWidth + 4;  // corrected typo: sprectralWidth
const int spectralHeight = meshHeight + 1;

const int L = 100;  // Patch size

static std::default_random_engine e;
static std::uniform_real_distribution<> dis(0, 1);

glm::vec2 windDir = glm::vec2(1.0f, 0.0f);
float V = 10.0f;
float A = 5.0f;

inline void checkCudaErrors(cudaError_t err, const char* file, const int line) {
  if (err != cudaSuccess) {
    std::cerr << "CUDA error at " << file << ":" << line
              << " code=" << static_cast<int>(err) << "("
              << cudaGetErrorString(err) << ")" << std::endl;
    // exit(EXIT_FAILURE);
  }
}
#define checkCudaErrors(err) (checkCudaErrors(err, __FILE__, __LINE__))

inline void checkCufftErrors(cufftResult err, const char* file,
                             const int line) {
  if (err != CUFFT_SUCCESS) {
    std::cerr << "CUFFT error at " << file << ":" << line
              << " code=" << static_cast<int>(err) << std::endl;
    // exit(EXIT_FAILURE);
  }
}

#define checkCufftErrors(err) (checkCufftErrors(err, __FILE__, __LINE__))

inline void checkGLError(const char* file, const int line) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error at " << file << ":" << line << " code=0x"
              << std::hex << err << std::endl;
  }
}

#define checkGLError() (checkGLError(__FILE__, __LINE__))

CudaWave::CudaWave() {
  initCuda();
  createMesh(meshWidth, meshHeight);
  createMeshIndexBuffer(meshWidth, meshHeight);
  compileShaders();
}

void CudaWave::initCuda() {
  // Initialize CUDA
  checkCudaErrors(cudaSetDevice(0));

  // Create CUFFT plan
  checkCufftErrors(cufftPlan2d(&plan, meshWidth, meshHeight, CUFFT_C2C));

  // Prepopulate the h0 array
  int spectrumSize = spectralWidth * spectralHeight;
  float2* h0 = (float2*)malloc(spectrumSize * sizeof(float2));
  if (!h0) {
    cerr << "Failed to allocate memory for h0" << endl;
    return;
  }
  generateInitialSpectrum(h0);

  // Allocate memory for GPU and copy h0 to it
  checkCudaErrors(cudaMalloc((void**)&cuda_h0, spectrumSize * sizeof(float2)));
  checkCudaErrors(cudaMemcpy(cuda_h0, h0, spectrumSize * sizeof(float2),
                             cudaMemcpyHostToDevice));
  free(h0);

  // Allocate memory for GPU output textures
  int outputSize = meshWidth * meshHeight;
  checkCudaErrors(cudaMalloc((void**)&cuda_ht, outputSize * sizeof(float2)));

  // Create VBOs for height and slope
  glGenBuffers(1, &height_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, height_vbo);
  glBufferData(GL_ARRAY_BUFFER, outputSize * sizeof(float), NULL,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &slope_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, slope_vbo);
  glBufferData(GL_ARRAY_BUFFER, outputSize * sizeof(float), NULL,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Register VBOs with CUDA -- CUDA can only write to VBOs
  checkCudaErrors(
      cudaGraphicsGLRegisterBuffer(&cuda_heightVB_resource, height_vbo,
                                   cudaGraphicsRegisterFlagsWriteDiscard));

  checkCudaErrors(cudaGraphicsGLRegisterBuffer(
      &cuda_slopeVB_resource, slope_vbo, cudaGraphicsMapFlagsWriteDiscard));
}

CudaWave::~CudaWave() {
  glDeleteBuffers(1, &cube_vbo);
  glDeleteBuffers(1, &pos_vbo);
  glDeleteBuffers(1, &ibo);
  glDeleteVertexArrays(1, &vao);
  shader->Delete();

  checkCudaErrors(cudaFree(cuda_h0));
  checkCudaErrors(cudaFree(cuda_ht));
  checkCudaErrors(cudaGraphicsUnregisterResource(cuda_heightVB_resource));
  checkCudaErrors(cudaGraphicsUnregisterResource(cuda_slopeVB_resource));
  glDeleteBuffers(1, &height_vbo);
  glDeleteBuffers(1, &slope_vbo);

  checkCufftErrors(cufftDestroy(plan));
}

void CudaWave::compileShaders() {
  shader = new Shader("ocean.vs", "ocean.fs");
  checkGLError();
}

void CudaWave::createMesh(int width, int height) {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &pos_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
  glBufferData(GL_ARRAY_BUFFER, width * height * 4 * sizeof(float), NULL,
               GL_DYNAMIC_DRAW);

  float* vertices = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  if (!vertices) {
    cerr << "Error mapping buffer" << endl;
    return;
  }

  float scale = 2.0f;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      float u = x / (float)(width - 1);
      float v = y / (float)(height - 1);

      *vertices++ = scale * u - 1.0f;
      *vertices++ = 0.0f;
      *vertices++ = scale * v - 1.0f;
      *vertices++ = 1.0f;
    }
  }

  glUnmapBuffer(GL_ARRAY_BUFFER);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CudaWave::createMeshIndexBuffer(int w, int h) {
  int size = ((w * 2) + 2) * (h - 1) * sizeof(GLuint);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);

  GLuint* indices =
      (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
  if (!indices) {
    cerr << "Error mapping buffer" << endl;
    return;
  }

  for (int y = 0; y < h - 1; ++y) {
    for (int x = 0; x < w; ++x) {
      *indices++ = y * w + x;
      *indices++ = (y + 1) * w + x;
    }
    *indices++ = (y + 1) * w + (w - 1);
    *indices++ = (y + 1) * w;
  }

  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CudaWave::renderMesh(Camera* camera) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader->Bind();

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = camera->GetViewMatrix();
  glm::mat4 proj = camera->GetProjectionMatrix();

  glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE,
                     glm::value_ptr(model));
  glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                     glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(shader->ID, "proj"), 1, GL_FALSE,
                     glm::value_ptr(proj));

  glUniform4f(glGetUniformLocation(shader->ID, "deepColor"), 0.0f, 0.1f, 0.4f,
              1.0f);
  glUniform4f(glGetUniformLocation(shader->ID, "shallowColor"), 0.1f, 0.3f,
              0.3f, 1.0f);
  glUniform4f(glGetUniformLocation(shader->ID, "skyColor"), 1.0f, 1.0f, 1.0f,
              1.0f);
  glUniform3f(glGetUniformLocation(shader->ID, "lightDir"), 0.0f, 1.0f, 0.0f);

  glUniform1f(glGetUniformLocation(shader->ID, "scale"), 0.1f);
  glUniform2f(glGetUniformLocation(shader->ID, "size"), (float)meshWidth,
              (float)meshHeight);

  // Bind VAO
  glBindVertexArray(vao);

  // Ensure the position VBO is also bound
  glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);

  // Bind and enable height VBO
  glBindBuffer(GL_ARRAY_BUFFER, height_vbo);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(1);

  // Bind and enable slope VBO
  glBindBuffer(GL_ARRAY_BUFFER, slope_vbo);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(2);

  // Bind index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  // Draw elements using the index buffer
  glDrawElements(GL_TRIANGLE_STRIP, ((meshWidth * 2) + 2) * (meshHeight - 1),
                 GL_UNSIGNED_INT, 0);

  // Unbind VAO and shader
  glBindVertexArray(0);
  shader->Unbind();
}

void CudaWave::update(float t, Camera* camera) {
  // Time dependent wave simulation
  // cout << "Time: " << t << endl;
  cudaGenerateTimeSpectrumKernel(cuda_h0, cuda_ht, spectralWidth, meshWidth,
                                 meshHeight, t, L);
  // Perform iFFT to map to spatial domain
  checkCufftErrors(cufftExecC2C(plan, cuda_ht, cuda_ht, CUFFT_INVERSE));

  // Calculate height and slope
  size_t num_bytes;
  checkCudaErrors(cudaGraphicsMapResources(1, &cuda_heightVB_resource, 0));
  checkCudaErrors(cudaGraphicsResourceGetMappedPointer(
      (void**)&height_ptr, &num_bytes, cuda_heightVB_resource));

  cudaUpdateHeightmapKernel(height_ptr, cuda_ht, meshWidth, meshHeight);
  checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_heightVB_resource, 0));
  // cout << "Heightmap updated" << endl;
  checkCudaErrors(cudaGraphicsMapResources(1, &cuda_slopeVB_resource, 0));
  checkCudaErrors(cudaGraphicsResourceGetMappedPointer(
      (void**)&slope_ptr, &num_bytes, cuda_slopeVB_resource));
  cudaCalculateSlopeKernel(height_ptr, slope_ptr, meshWidth, meshHeight);

  checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_slopeVB_resource, 0));
  cout << "Slopes calculated" << endl;
  // Render mesh
  renderMesh(camera);
}

static inline float exponent(float num, int exp) {
  float res = 1.0f;
  if (exp < 0) {
    cerr << "Error: exponent must be greater than or equal to 0" << endl;
    return 0.0f;
  }
  for (int i = 0; i < exp; ++i) {
    res *= num;
  }
  return res;
}

static inline float urand() {
  float u = (float)dis(e);
  if (u < 1e-6f) {
    u = 1e-6f;
  }
  return u;
}

static inline float guassRN() {
  float u = urand();
  float v = urand();
  return sqrt(-2.0f * log(u)) * cos(2.0f * M_PI * v);
}

static inline float Phillips(glm::vec2 K, glm::vec2 wind_dir, float V,
                             float A) {
  float k_2 = K.x * K.x + K.y * K.y;
  if (k_2 < 0.00001f) {
    return 0.0f;
  }
  float L = V * V / M_GRAVITY;
  float k_dot_w = glm::dot(glm::normalize(K), glm::normalize(wind_dir));
  float P = A * exp(-1.0f / (k_2 * L * L)) / (k_2 * k_2) * exponent(k_dot_w, 4);
  if (k_dot_w < 0.0f) {
    P *= 0.07f;
  }
  return P;
}

void CudaWave::generateInitialSpectrum(float2* h0) {
  for (unsigned int y = 0; y <= meshHeight; ++y) {
    for (unsigned int x = 0; x <= meshWidth; ++x) {
      glm::vec2 k =
          glm::vec2((int)(meshWidth / 2.0f) + x, (int)(meshHeight / 2.0f) + y);
      k *= (2.0f * M_PI / L);
      float P = 0.0f;
      if (k.x != 0.0f || k.y != 0.0f) {
        P = sqrt(Phillips(k, windDir, V, A));
      }
      float Er = guassRN();
      float Ei = guassRN();
      float real = (Er * P) * sqrt(0.5f);
      float imag = (Ei * P) * sqrt(0.5f);

      int i = y * spectralWidth + x;
      int maxSize = spectralWidth * spectralHeight;
      h0[i].x = real;
      h0[i].y = imag;
    }
  }
  cout << "Initial spectrum generated" << endl;
}
*/
