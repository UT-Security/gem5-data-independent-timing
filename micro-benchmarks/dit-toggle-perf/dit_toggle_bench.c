#include <stdio.h>
#include <stdint.h>
#include <gem5/m5ops.h>

#define ITERATIONS 100000

// Baseline: arithmetic operations without DIT toggling
uint64_t baseline_test(uint64_t seed) {
    uint64_t a = seed;
    uint64_t b = seed + 1;

    for (uint64_t i = 0; i < ITERATIONS; i++) {
        // Junk instruction (matching MSR DIT location in toggle tests)
        __asm__ volatile("add %0, %0, xzr" : "+r" (a));

        // Arithmetic operations
        a = a + b;
        b = b * 3;

        // Junk instruction (matching MSR DIT location in toggle tests)
        __asm__ volatile("add %0, %0, xzr" : "+r" (b));

        // Arithmetic operations
        a = a ^ b;
        b = b - a;
    }

    return a + b;
}

// Test: Same arithmetic operations with DIT toggling
uint64_t dit_toggle_test(uint64_t seed) {
    uint64_t a = seed;
    uint64_t b = seed + 1;

    for (uint64_t i = 0; i < ITERATIONS; i++) {
        // Turn DIT ON (immediate form)
        __asm__ volatile("msr DIT, #1");

        // Arithmetic operations with DIT enabled
        a = a + b;
        b = b * 3;

        // Turn DIT OFF (immediate form)
        __asm__ volatile("msr DIT, #0");

        // Arithmetic operations with DIT disabled
        a = a ^ b;
        b = b - a;
    }

    return a + b;
}

// Test: DIT toggling using register form
uint64_t dit_toggle_reg_test(uint64_t seed) {
    uint64_t a = seed;
    uint64_t b = seed + 1;
    uint64_t dit_on = 1UL << 24;   // DIT bit in position 24
    uint64_t dit_off = 0UL;

    for (uint64_t i = 0; i < ITERATIONS; i++) {
        // Turn DIT ON (register form)
        __asm__ volatile("msr DIT, %0" :: "r" (dit_on));

        // Arithmetic operations with DIT enabled
        a = a + b;
        b = b * 3;

        // Turn DIT OFF (register form)
        __asm__ volatile("msr DIT, %0" :: "r" (dit_off));

        // Arithmetic operations with DIT disabled
        a = a ^ b;
        b = b - a;
    }

    return a + b;
}

int main(int argc, char *argv[]) {
    uint64_t result;
    uint64_t seed = 0x12345678;

    printf("=== DIT Toggle Performance Benchmark ===\n");
    printf("Iterations: %d\n\n", ITERATIONS);

    // Baseline test (no DIT toggling)
    printf("Running baseline test (no DIT toggling)...\n");
    m5_reset_stats(0, 0);  // Reset stats before baseline
    result = baseline_test(seed);
    m5_dump_stats(0, 0);  // Dump baseline stats
    printf("  Result: 0x%lx\n\n", result);

    // DIT toggle test with immediate form
    printf("Running DIT toggle test (immediate form)...\n");
    m5_reset_stats(0, 0);  // Reset stats before immediate test
    result = dit_toggle_test(seed);
    m5_dump_stats(0, 0);  // Dump immediate test stats
    printf("  Result: 0x%lx\n\n", result);

    // DIT toggle test with register form
    printf("Running DIT toggle test (register form)...\n");
    m5_reset_stats(0, 0);  // Reset stats before register test
    result = dit_toggle_reg_test(seed);
    m5_dump_reset_stats(0, 0);  // Dump register test stats
    printf("  Result: 0x%lx\n\n", result);

    printf("=== Benchmark Complete ===\n");
    printf("Check stats.txt for cycle counts and performance comparison\n");

    return 0;
}
