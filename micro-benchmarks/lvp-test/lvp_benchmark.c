/*
 * LVP Micro-benchmark
 *
 * Designed to demonstrate Load Value Prediction benefits:
 * - Integer-only loads (no vectors)
 * - Predictable/constant load values
 * - Dependent instruction chains that benefit from speculation
 *
 * Compile: gcc -O2 -static lvp_benchmark.c -o lvp_benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_ITERATIONS 1000

// Structure with predictable fields
// NOTE: Padding added to prevent ldp (load pair) instruction generation
// LVP only handles single-destination loads (numDestRegs == 1)
typedef struct Node {
    int64_t value;        // Constant values
    int64_t padding1[2];  // Padding to prevent ldp pairing
    int64_t multiplier;   // Mostly constant (1, 2, or 4)
    int64_t padding2[2];  // Padding to prevent ldp pairing
    int64_t offset;       // Constant offset
    int64_t padding3[2];  // Padding to prevent ldp pairing
    struct Node *next;    // Pointer chain
} Node;

// Global arrays to avoid stack issues
Node nodes[ARRAY_SIZE];
int64_t results[ARRAY_SIZE];

// Index array for dependent loads - creates serial dependency chain
// Each load's address depends on previous load's value
int64_t index_array[ARRAY_SIZE];
int64_t data_array[ARRAY_SIZE];

void initialize_data() {
    // Initialize nodes with predictable patterns
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Most loads will return constant values
        nodes[i].value = 42;           // CONSTANT load value
        nodes[i].multiplier = 2;       // CONSTANT multiplier
        nodes[i].offset = 100;         // CONSTANT offset

        // Create circular linked list
        nodes[i].next = &nodes[(i + 1) % ARRAY_SIZE];

        // Add some variation (10% of nodes)
        if (i % 10 == 0) {
            nodes[i].value = 43;       // Slightly different but still predictable
            nodes[i].multiplier = 4;
        }
    }

    // Initialize index chain for dependent loads
    // All values are the same constant - maximizes LVP effectiveness
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Every position contains the same constant value
        index_array[i] = 1;  // Always returns 1 (CONSTANT)

        // Data array also contains constant values
        data_array[i] = 1;   // Always returns 1 (CONSTANT)
    }
}

// Benchmark 1: Serial dependent loads (critical for LVP)
// Each load's address depends on previous load's value
// Creates true RAW dependency chain that only LVP can break
int64_t test_constant_loads() {
    int64_t sum = 0;
    int64_t idx = 0;  // Starting index

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            // Load 1: Get index from index_array
            // This load determines the address of Load 2
            int64_t idx1 = index_array[idx];        // CONSTANT: always same idx1 for same idx

            // Load 2: Use idx1 to load from data_array
            // This load depends on Load 1's value (RAW dependency)
            // Without LVP: must wait for idx1 before computing address
            // With LVP: idx1 predicted, address computed speculatively
            int64_t idx2 = data_array[idx1];        // CONSTANT: always same for same idx1

            // Load 3: Use idx2 to load again
            // This load depends on Load 2's value (serial dependency)
            int64_t idx3 = data_array[idx2];        // CONSTANT: always same for same idx2

            // Simple accumulation - each add depends only on one load value
            // This creates clean single-source dependencies
            sum += idx1;
            sum += idx2;
            sum += idx3;

            // Update index for next iteration
            idx = (idx + 1) % ARRAY_SIZE;
        }
    }

    return sum;
}

// Benchmark 2: Array lookups with predictable indices
int64_t test_predictable_indices() {
    int64_t sum = 0;

    // Index pattern that repeats
    int indices[16] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        for (int i = 0; i < 16; i++) {
            int idx = indices[i];  // Predictable pattern

            // Load from predictable location
            int64_t val = nodes[idx].value;
            int64_t mult = nodes[idx].multiplier;

            // Dependent arithmetic
            sum += val * mult;
        }
    }

    return sum;
}

// Benchmark 3: Nested dependent loads
int64_t test_nested_dependencies() {
    int64_t sum = 0;

    for (int iter = 0; iter < NUM_ITERATIONS / 10; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            // Load chain with dependencies
            int64_t a = nodes[i].value;              // Load 1 - CONSTANT
            int64_t b = nodes[a % ARRAY_SIZE].value; // Load 2 - depends on load 1
            int64_t c = nodes[b % ARRAY_SIZE].value; // Load 3 - depends on load 2

            // All these loads will be predictable (always 42 or 43)
            // With LVP: all three loads predicted speculatively
            // Without LVP: serial dependency chain, lots of stalls
            sum += a + b + c;
        }
    }

    return sum;
}

int main() {
    // printf("=== LVP Micro-benchmark ===\n");
    // printf("Array size: %d\n", ARRAY_SIZE);
    // printf("Iterations: %d\n\n", NUM_ITERATIONS);

    // Initialize data
    initialize_data();

    // printf("Running benchmarks...\n");

    // Benchmark 1: Constant loads
    int64_t result1 = test_constant_loads();
    printf("Test 1 (Constant Loads): Result = %ld\n", result1);

    // Benchmark 2: Predictable indices
    // int64_t result2 = test_predictable_indices();
    // printf("Test 2 (Predictable Indices): Result = %ld\n", result2);

    // // Benchmark 3: Nested dependencies
    // int64_t result3 = test_nested_dependencies();
    // printf("Test 3 (Nested Dependencies): Result = %ld\n", result3);

    printf("\n=== Benchmark Complete ===\n");
    printf("Total result checksum: %ld\n", result1);

    return 0;
}
