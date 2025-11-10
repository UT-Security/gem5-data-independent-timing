#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100

// This benchmark focuses on memory-bound operations with high silent store rates
// to maximize the benefit of silent store elimination

int main() {
    // Large arrays to stress memory system
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* shadow = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* flags = (int*)malloc(ARRAY_SIZE * sizeof(int));

    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
        shadow[i] = data[i];
        flags[i] = 0;
    }

    printf("Starting memory-intensive silent store benchmark...\n");

    // Main benchmark loop
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Phase 1: Conditional updates (many will be silent)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int old_val = data[i];
            int new_val = (old_val * 7 + 3) % 100;

            // Often writes back the same value due to modulo
            if (new_val < 50) {
                data[i] = old_val;  // Silent store
            } else {
                data[i] = new_val;  // Non-silent store
            }
        }

        // Phase 2: Shadow array synchronization (100% silent on even iterations)
        if (iter % 2 == 0) {
            // Copy data to shadow - non-silent stores
            for (int i = 0; i < ARRAY_SIZE; i++) {
                shadow[i] = data[i];
            }
        } else {
            // Write shadow back to shadow - 100% silent stores
            for (int i = 0; i < ARRAY_SIZE; i++) {
                shadow[i] = shadow[i];  // Completely silent
            }
        }

        // Phase 3: Flag updates with redundancy
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int flag_val = (data[i] > 50) ? 1 : 0;
            flags[i] = flag_val;  // Often silent after first iteration
        }

        // Phase 4: Sparse updates creating memory pressure
        for (int i = 0; i < ARRAY_SIZE; i += 64) {  // Cache line stride
            data[i] = data[i];  // 100% silent stores at cache line boundaries
        }
    }

    // Verify results
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += data[i] + shadow[i] + flags[i];
    }

    printf("Benchmark completed. Checksum: %d\n", checksum);

    free(data);
    free(shadow);
    free(flags);

    return 0;
}