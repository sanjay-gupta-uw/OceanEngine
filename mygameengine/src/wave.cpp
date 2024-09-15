#include "wave.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb\stb_image_write.h"

using namespace glm;
using namespace std;

int N = 256;   // Number of (sin) waves
int L = 1000;  // Patch size

Wave::Wave() {
  // initialize wave parameters
  A = 4.0f;
  g = 9.81f;
  v = 10.0f;

  // mesh creation
  vertices = new glm::vec3[N * N];
  texCoords = new glm::vec2[N * N];

  createSurface();

  h0_k_ = new std::complex<float>[N * N];
  h_kt_ = new std::complex<float>[N * N];
  generatePhillipsSpectrum();

  float currentFrame = 0.0f;
  float lastFrame = 0.0f;
  float deltaTime = 0.0f;
  float timeStep = 0.0f;  // This is the 't' value used for ocean simulation
}

Wave::~Wave() {
  // free memory
  delete[] h0_k_;
}

void Wave::setCamera(Camera *camera) { this->camera = camera; }

void Wave::setShader(Shader *shader) {
  this->shader = shader;
  initRenderParams();
}

void Wave::initRenderParams() {
  glGenVertexArrays(1, &VAO);  // Generate VAO
  glGenBuffers(1, &VBO);       // Generate VBO
  glGenBuffers(1, &EBO);       // Generate EBO
  glGenBuffers(1, &texVBO);    // Generate VBO for texture coordinates

  // Bind VAO
  glBindVertexArray(VAO);

  // Bind VBO and send vertex data to GPU
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, N * N * sizeof(glm::vec3), vertices,
               GL_STATIC_DRAW);

  // Bind EBO and send index data to GPU
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  // Specify vertex attributes (position attribute)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, texVBO);
  glBufferData(GL_ARRAY_BUFFER, N * N * sizeof(glm::vec2), texCoords,
               GL_STATIC_DRAW);

  // Specify the texture coordinates attribute (location 1 in shader)
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
  glEnableVertexAttribArray(1);

  // Unbind the VAO (safe practice)
  glBindVertexArray(0);
}

void Wave::createSurface() {
  const int planeSize = 1000;
  const int shift = planeSize / 2;

  float step = planeSize / (N - 1);
  // Create vertices
  for (int z = 0; z < N; ++z) {
    for (int x = 0; x < N; ++x) {
      vertices[z * N + x] = vec3(x * step - shift, 0.0f, z * step - shift);
    }
  }

  // Create indices
  for (int z = 0; z < N - 1; ++z) {
    for (int x = 0; x < N - 1; ++x) {
      // Indices for the first triangle of the quad
      indices.push_back(z * N + x);        // top-left
      indices.push_back((z + 1) * N + x);  // bottom-left
      indices.push_back(z * N + (x + 1));  // top-right

      // Indices for the second triangle of the quad
      indices.push_back((z + 1) * N + x);        // bottom-left
      indices.push_back((z + 1) * N + (x + 1));  // bottom-right
      indices.push_back(z * N + (x + 1));        // top-right
    }
  }

  float texStep = 1.0f / (N - 1);  // Step size for texture coordinates
  for (int z = 0; z < N; ++z) {
    for (int x = 0; x < N; ++x) {
      vertices[z * N + x] = vec3(x * step - shift, 0.0f, z * step - shift);
      texCoords[z * N + x] =
          vec2(x * texStep, z * texStep);  // Set texture coordinates
    }
  }
}

float Wave::Phillips(glm::vec2 K) {
  glm::vec2 windDir = vec2(1.0f, 1.0f);

  float waveMagnitude = length(K);  // since K is our wave vector

  if (waveMagnitude < 0.0001f) return 0.0f;
  float L = v * v / g;
  return float(
      A * float(std::exp(-1.0f / std::pow(waveMagnitude * L, 2))) /
      std::pow(waveMagnitude, 4) *
      std::pow(glm::dot(glm::normalize(windDir), glm::normalize(K)), 8));
}

void Wave::generatePhillipsSpectrum() {
  cout << "Generating Phillips Spectrum" << endl;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<float> distribution(0.0f, 1.0f);

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      float n = float(j - N / 2);
      float m = float(i - N / 2);

      glm::vec2 K = glm::vec2(2.0f * glm::pi<float>() * n / L,
                              2.0f * glm::pi<float>() * m / L);
      float guass_real = distribution(gen);
      float guass_img = distribution(gen);

      float P = std::sqrt(Phillips(K) * 0.5f);
      h0_k_[i * N + j] = std::complex<float>(guass_real * P, guass_img * P);
    }
  }

  cout << "Generated Phillips Spectrum" << endl;
  saveAsImage(2.0f);  // Call save with a brightness scale factor
}

// Should be computed on the GPU
void Wave::generateH_KT_Spectrum(float t) {
  // Generate h_kt from h0_k_
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      // Calculate wave vector K and -K
      glm::vec2 K_indices =
          glm::vec2(j - N / 2, i - N / 2);  // Original indices
      glm::vec2 minus_K_indices =
          glm::vec2(N - 1 - j, N - 1 - i);  // Indices for -K

      // Get h0(K) and h0(-K)
      std::complex<float> h0_K = h0_k_[i * N + j];  // Get h0(K)
      std::complex<float> h0_minusK =
          h0_k_[(int(minus_K_indices.y)) * N +
                int(minus_K_indices.x)];  // Get h0(-K)

      // Calculate the angular frequency w(k) (dispersion relation)
      float w_k = std::sqrt(
          9.81 *
          length(K_indices));  // Example: w(k) = sqrt(g * |k|), g = 9.81 m/s�

      // Calculate the time-dependent Fourier amplitudes
      std::complex<float> exp_iwt =
          std::exp(std::complex<float>(0, w_k * t));  // e^(i * w(k) * t)
      std::complex<float> exp_neg_iwt =
          std::exp(std::complex<float>(0, -w_k * t));  // e^(-i * w(k) * t)

      // Compute h_kt_ at this K
      h_kt_[i * N + j] = h0_K * exp_iwt + std::conj(h0_minusK) * exp_neg_iwt;
    }
  }
  // saveAsImage(2.0f, 1);  // Call save with a brightness scale factor
}

// Function to perform the 2D Inverse FFT and generate the height field
void Wave::generateHeightField() {
  // Create arrays to store the spatial domain height field
  fftwf_complex *h_kt_complex =
      fftwf_alloc_complex(N * N);  // FFTW input (frequency domain)
  fftwf_complex *height_field =
      fftwf_alloc_complex(N * N);  // FFTW output (spatial domain)

  // Copy h_kt_ (frequency domain) data to h_kt_complex
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      h_kt_complex[i * N + j][0] = h_kt_[i * N + j].real();  // Real part
      h_kt_complex[i * N + j][1] = h_kt_[i * N + j].imag();  // Imaginary part
    }
  }

  // Create an FFTW plan to compute the 2D IFFT
  fftwf_plan ifft_plan = fftwf_plan_dft_2d(N, N, h_kt_complex, height_field,
                                           FFTW_BACKWARD, FFTW_ESTIMATE);

  // Execute the IFFT
  fftwf_execute(ifft_plan);

  // Normalize the result (scaling after FFTW IFFT)
  float N2 = N * N;
  float *real_part = new float[N2];  // Real part of the height field
  for (int i = 0; i < N2; ++i) {
    // height_field[i][0] /= (N2);  // Normalize real part
    // height_field[i][1] /= (N2);  // Normalize imaginary part (though it
    // should be very small)
    real_part[i] = height_field[i][0] / (N2);  // Store the real part
  }

  // Free the FFTW plan and input/output arrays
  fftwf_destroy_plan(ifft_plan);
  fftwf_free(h_kt_complex);

  // Save the height field as an image
  // saveHeightFieldAsImage(height_field);

  // Free the height field
  fftwf_free(height_field);

  glGenTextures(1, &heightMapTexture);
  glBindTexture(GL_TEXTURE_2D, heightMapTexture);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload the height field data (use real_part array as the source of height
  // data)
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, N, N, 0, GL_RED, GL_FLOAT, real_part);

  // Generate mipmaps for better scaling
  glGenerateMipmap(GL_TEXTURE_2D);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Wave::update() {
  currentFrame = static_cast<float>(glfwGetTime());
  deltaTime = currentFrame - lastFrame;
  timeStep += deltaTime;
  // update wave
  // Take h0_k_ and generate time dependent component, h_kt
  generateH_KT_Spectrum(timeStep);
  generateHeightField();
  render();
  // cout << "Updated Wave" << endl;
  lastFrame = currentFrame;
}

void Wave::render() {
  // Use the shader program
  shader->Bind();
  // Bind the VAO (this also binds the VBO and EBO stored in the VAO)
  glBindVertexArray(VAO);

  // Bind the height map texture
  glActiveTexture(GL_TEXTURE0);  // Activate texture unit 0
  glBindTexture(GL_TEXTURE_2D,
                heightMapTexture);  // Bind the height map texture
  glUniform1i(glGetUniformLocation(shader->ID, "heightMap"),
              0);  // Pass texture to shader

  // Set the height scale uniform (controls how much the vertices are displaced)
  glUniform1f(glGetUniformLocation(shader->ID, "heightScale"), 40.0f);

  // pass view and projection matrices to the shader
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = camera->GetViewMatrix();
  glm::mat4 projection = camera->GetProjectionMatrix();

  // Set the model, view, and projection matrices in the shader
  glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE,
                     &model[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                     &view[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                     GL_FALSE, &projection[0][0]);

  // set light and color uniforms for fragment shader
  // MAKE LIGHT POSITION DYNAMIC
  glUniform3f(glGetUniformLocation(shader->ID, "lightPosition"), 0.0f, 100.0f,
              0.0f);
  glUniform3f(glGetUniformLocation(shader->ID, "lightColor"), 1.0f, 1.0f, 1.0f);
  glUniform3f(glGetUniformLocation(shader->ID, "waterColorDeep"), 0.0f, 0.0f,
              0.5f);
  glUniform3f(glGetUniformLocation(shader->ID, "waterColorShallow"), 0.0f, 0.5f,
              1.0f);

  // Draw the plane (use indices to render triangles)
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

  // Unbind VAO (optional, generally safe practice)
  glBindVertexArray(0);

  // Unbind shader program (optional, generally safe practice)
  glUseProgram(0);
}

void Wave::saveAsImage(float brightnessScale, int option) {
  // Prepare arrays to hold real and imaginary parts for normalization
  float *realPart = new float[N * N];
  float *imagPart = new float[N * N];

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      std::complex<float> h0 = h0_k_[i * N + j];
      realPart[i * N + j] =
          std::log1p(std::abs(h0.real()));  // Logarithmic scaling for real part
      imagPart[i * N + j] = std::log1p(
          std::abs(h0.imag()));  // Logarithmic scaling for imaginary part
    }
  }

  // Normalize real part
  float realMax = *std::max_element(realPart, realPart + N * N);
  float realMin = *std::min_element(realPart, realPart + N * N);
  unsigned char *image_data =
      new unsigned char[N * N * 3];  // 3 channels: R, G, B

  for (int i = 0; i < N * N; ++i) {
    // Apply brightness scale to the real and imaginary parts
    float normalizedReal = (realPart[i] - realMin) / (realMax - realMin) *
                           255.0f * brightnessScale;
    float normalizedImag = (imagPart[i] - realMin) / (realMax - realMin) *
                           255.0f * brightnessScale;

    // Clamp values between 0 and 255
    normalizedReal = std::min(255.0f, std::max(0.0f, normalizedReal));
    normalizedImag = std::min(255.0f, std::max(0.0f, normalizedImag));

    // Set the red and green channels to the real part and the blue channel
    // to the imaginary part
    image_data[i * 3 + 0] =
        static_cast<unsigned char>(normalizedReal);  // Red channel
    image_data[i * 3 + 1] =
        static_cast<unsigned char>(normalizedImag);         // Green channel
    image_data[i * 3 + 2] = static_cast<unsigned char>(0);  // Blue channel
  }

  // Save the image as a PNG file

  stbi_write_png(option == 0 ? "results/phillips_spectrum.png"
                             : "results/phillips_spectrum_time.png",
                 N, N, 3, image_data, N * 3);

  // Free the allocated memory
  delete[] image_data;
  delete[] realPart;
  delete[] imagPart;

  if (option == 0) {
    cout << "Saved Phillips Spectrum" << endl;
  } else {
    cout << "Saved Phillips Spectrum with time" << endl;
  }
}

void Wave::saveHeightFieldAsImage(fftwf_complex *height_field) {
  // Allocate memory for a single-channel grayscale image
  unsigned char *image_data = new unsigned char[N * N];  // For grayscale image
  float *real_part = new float[N * N];

  // Extract the real part from the height field (height should be real)
  for (int i = 0; i < N * N; ++i) {
    real_part[i] =
        height_field[i][0];  // Use only the real part for the heightfield
  }

  // Normalize the real part to [0, 255] for image representation
  float real_max = *std::max_element(real_part, real_part + N * N);
  float real_min = *std::min_element(real_part, real_part + N * N);

  for (int i = 0; i < N * N; ++i) {
    // Normalize real part to the range [0, 255]
    float normalized_value =
        (real_part[i] - real_min) / (real_max - real_min) * 255.0f;
    normalized_value =
        std::min(255.0f, std::max(0.0f, normalized_value));  // Clamp values
    image_data[i] =
        static_cast<unsigned char>(normalized_value);  // Set grayscale value
  }

  // Save the image as a grayscale PNG (1 channel)
  stbi_write_png("results/height_field_grayscale.png", N, N, 1, image_data, N);

  // Free allocated memory
  delete[] real_part;
  delete[] image_data;

  cout << "Saved height field as a grayscale image." << endl;
}
