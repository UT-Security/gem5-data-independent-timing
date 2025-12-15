#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <gem5/m5ops.h>

#define SIZE_Y 5
#define SIZE_X 5
#define CHANNELS 4

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

__attribute__((noinline))
void compute(int32_t* kernel, uint8_t pixel, int32_t* sum) {
    // Flood multiply unit right before the timed multiplies
    flood_multiply(10);

    for (int32_t y = 0; y < SIZE_Y; y++) {
        for (int32_t x = 0; x < SIZE_X; x++) {
            for (int32_t c = 0; c < CHANNELS; c++) {
                // madd: sum[c] = kernel[y*SIZE_X + x] * pixel + sum[c]
                sum[c] += kernel[y * SIZE_X + x] * pixel;
            }
        }
    }

    // Barrier
    __asm__ volatile("dsb sy\n\t isb" ::: "memory");
}

int main() {
    setbuf(stdout, NULL);

    int32_t kernel[SIZE_Y * SIZE_X] = {2, 2, 2, 2, 2, 2, 2, 2, 2};
    int32_t sum[CHANNELS];

    // Number of iterations to amplify timing
    const int32_t iterations = 100000;
    const int32_t runs = 2;

    uint64_t start, end;

    for (int32_t r = 0; r < runs; r++) {
        // Test with pixel = 0 (should be fast - trivial multiply)
        for (int i = 0; i < CHANNELS; i++) sum[i] = 0;
        start = m5_rpns();
        for (int32_t i = 0; i < iterations; i++) {
            compute(kernel, 0, sum);
        }
        end = m5_rpns();
        printf("Run %d: Pixel=0:   %.3f us\n", r, (end - start) / 1e6);
    }

    for (int32_t r = 0; r < runs; r++) {
        // Test with pixel = 255 (should be slow - normal multiply)
        for (int i = 0; i < CHANNELS; i++) sum[i] = 0;
        start = m5_rpns();
        for (int32_t i = 0; i < iterations; i++) {
            compute(kernel, 255, sum);
        }
        end = m5_rpns();
        printf("Run %d: Pixel=255: %.3f us\n", r, (end - start) / 1e6);
    }

    return 0;
}
