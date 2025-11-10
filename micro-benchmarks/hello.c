#include <stdio.h>
#include <gem5/m5ops.h>

#define ITERATIONS 1000

int main(int argc, char *argv[]) {
    long long sum1 = 0, sum2 = 0;
    long long factor1 = 3, factor2 = 7;

    for (int i = 0; i < ITERATIONS; i++) {
        asm volatile("msr DIT, #1");

        // Independent operations
        asm volatile(
            "mul %0, %0, %1\n\t"
            "add %0, %0, %1\n\t"
            : "+r"(sum1)
            : "r"(factor1)
        );

        asm volatile("msr DIT, #0");

        asm volatile(
            "mul %0, %0, %1\n\t"
            "add %0, %0, %1\n\t"
            : "+r"(sum2)
            : "r"(factor2)
        );
    }

    printf("Final sum1: %lld\n", sum1);
    printf("Final sum2: %lld\n", sum2);

    return 0;
}
