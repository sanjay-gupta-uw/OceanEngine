![Alt text](./renders/ocean.png?raw=true "Optional Title")

# README

This project was developed for fun to study rendering complex phenomena on the computer. This is still a work on progress, but the scene renders the moving ocean. Essentially, the algorithm works as follows: given a 2D plane of points, we can displace each point's y-coordinate in the vertex shader according to a height texture generated each frame representing the ocean height field. This texture is obtained by inverse discrete fourier transform (currently using FFTW) based on frequency inputs according to the Phillips Spectrum.

# Left to Implement

- Choppy Waves
- Update fragment shader to colour ocean realistically
- Foam
- GPU Rendering options (CUDA)
- LOD

## References

https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__OPENGL.html#group__CUDART__OPENGL
https://github.com/ndd314/cuda_examples/blob/master/5_Simulations/oceanFFT/oceanFFT.cpp

https://www.diva-portal.org/smash/get/diva2:1778248/FULLTEXT02
file:///C:/Users/sanja/Downloads/GPGPU_FFT_Ocean_Simulation.pdf
https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/OceanCS_Slides.pdf

https://docs.nvidia.com/cuda/cufft/
https://arm-software.github.io/opengl-es-sdk-for-android/ocean_f_f_t.html
https://arm-software.github.io/opengl-es-sdk-for-android/fftwater_8cpp_source.html
https://slembcke.github.io/WaterWaves (Cool Demo)
https://www.researchgate.net/publication/264839743_Simulating_Ocean_Water
https://david.li/waves/

Inspiration:
https://www.youtube.com/watch?v=U2fkrXxvPRY&ab_channel=JS
