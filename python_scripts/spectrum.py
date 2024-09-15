import numpy as np
from PIL import Image

N = 256  # Number of waves
patchSize = 1000  # Size of the patch in meters

# Generate grids of independent random Gaussian values for real and imaginary parts
gaussian_random_real = np.random.normal(0, 1, (N, N))
gaussian_random_imag = np.random.normal(0, 1, (N, N))

# K: wind vector
def phillips(K):
    A = 4
    g = 9.81
    v = 40

    L = v * v / g
    windDir = np.array([1, 1])

    # k = magnitude of the wavevector K
    k = np.linalg.norm(K)
    
    if k == 0:  # Skip the zero norm case to avoid division by zero
        return 0
    
    # Phillips spectrum formula
    return A * np.exp(-1 / (k * L) ** 2) / (k ** 4) * (np.dot(K / k, windDir / np.linalg.norm(windDir))) ** 2

# Initialize the wave spectrum
def initializeSpectrum():
    print("Initializing spectrum")
    spectrum_real = np.zeros((N, N))
    spectrum_imag = np.zeros((N, N))

    domain = range(int(-N / 2), int(N / 2), 1)
    for n in domain:
        for m in domain:
            K = np.array([n * 2 * np.pi / patchSize, m * 2 * np.pi / patchSize])  # can optimize this
            if np.linalg.norm(K) == 0:  # Skip the zero norm case to avoid division by zero
                continue
            real_gaussian = gaussian_random_real[n + N // 2, m + N // 2]  # Use the random Gaussian value for the real part
            imag_gaussian = gaussian_random_imag[n + N // 2, m + N // 2]  # Use the random Gaussian value for the imaginary part

            P_h = np.sqrt(phillips(K) * 0.5)  # Phillips spectrum
            
            spectrum_real[n + N // 2, m + N // 2] = real_gaussian * P_h  # Real part
            spectrum_imag[n + N // 2, m + N // 2] = imag_gaussian * P_h  # Imaginary part

    # Clamp negative values to a small positive value (or zero)
    spectrum_real = np.where(spectrum_real < 0, 1e-10, spectrum_real)
    spectrum_imag = np.where(spectrum_imag < 0, 1e-10, spectrum_imag)
    
    return spectrum_real, spectrum_imag

# Save the spectrum as an RGB image
def saveSpectrumAsImage(spectrum_real, spectrum_imag, brightness_scale=2.0):
    # Logarithmic scaling for normalization
    spectrum_real_log = np.log1p(spectrum_real)  # Apply log scaling to the real part
    spectrum_real_log = np.nan_to_num(spectrum_real_log, nan=0.0)  # Replace NaN with 0

    spectrum_imag_log = np.log1p(spectrum_imag)  # Apply log scaling to the imaginary part
    spectrum_imag_log = np.nan_to_num(spectrum_imag_log, nan=0.0)  # Replace NaN with 0

    # Normalize the real part
    normalized_real = spectrum_real_log - np.min(spectrum_real_log)
    normalized_real /= np.max(normalized_real)
    normalized_real *= 255 * brightness_scale  # Apply brightness scaling
    normalized_real = np.clip(normalized_real, 0, 255).astype(np.uint8)  # Clip to 8-bit range

    # Normalize the imaginary part
    normalized_imag = spectrum_imag_log - np.min(spectrum_imag_log)
    normalized_imag /= np.max(normalized_imag)
    normalized_imag *= 255 * brightness_scale  # Apply brightness scaling
    normalized_imag = np.clip(normalized_imag, 0, 255).astype(np.uint8)  # Clip to 8-bit range

    # Create an RGB image where the real part is used for red and green channels, and imaginary for blue
    height, width = normalized_real.shape
    image = np.zeros((height, width, 3), dtype=np.uint8)

    # Populate the red and green channels with the normalized real spectrum
    image[:, :, 0] = normalized_real  # Red channel
    image[:, :, 1] = normalized_imag  # Green channel

    # Populate the blue channel with the normalized imaginary spectrum
    image[:, :, 2] = 0  # Blue channel

    # Convert to a PIL image and save
    img = Image.fromarray(image)
    img.save("../mygameengine/results/spectrum_image_brightened.png")
    print("Image saved as spectrum_image_brightened.png")

# Main program execution
spectrum_real, spectrum_imag = initializeSpectrum()
saveSpectrumAsImage(spectrum_real, spectrum_imag, brightness_scale=3.0)
