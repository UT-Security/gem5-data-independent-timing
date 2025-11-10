#include <stdio.h>
#include <stdint.h>

#define ITERATIONS 10000

// Volatile result to prevent dead code elimination
volatile uint64_t result = 0;

int main() {
    printf("Multiplication Chain Benchmark - Testing Computational Simplification\n");

    uint64_t val = 1;
    uint64_t m = 1;  // Multiplier value
    uint64_t count = ITERATIONS;

    // Hand-written assembly loop with 10 chained dependent multiplies
    // Each multiply depends on the previous result (true RAW dependency)
    // With comp simp: each multiply-by-1 should use fast-path (1-cycle)
    // Without comp simp: each uses normal MUL latency (3+ cycles)
    __asm__ __volatile__(
        "1:\n\t"
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "mul %[val], %[val], %[m]\n\t"   // val = val * m
        "subs %[count], %[count], #1\n\t"
        "b.ne 1b\n\t"
        : [val] "+r" (val), [count] "+r" (count)
        : [m] "r" (m)
        : "cc"
    );

    result = val;

    printf("Benchmark Complete: result = %lu\n", result);
    return 0;
}
