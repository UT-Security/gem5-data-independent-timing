#include <stdio.h>
#include <gem5/m5ops.h>

#define ITERATIONS 1000

int main(int argc, char *argv[]) {
    volatile int data[4] = {0, 1, 2, 3};

    printf("Starting silent store test...\n");

    // // Start simulation region
    // m5_roi_begin();

    for (int i = 0; i < ITERATIONS; i++) {
        // Toggle DIT bit
        if (i % 2 == 0) {
            //asm volatile("msr DIT, #1"); // Disable optimizations
        } else {
            //asm volatile("msr DIT, #0"); // Enable optimizations
        }

        // Perform silent stores - writing same values back
        data[0] = 0;  // Silent store (writing 0 to location containing 0)
        data[1] = 1;  // Silent store (writing 1 to location containing 1)
        data[2] = 2;  // Silent store (writing 2 to location containing 2)
        data[3] = 3;  // Silent store (writing 3 to location containing 3)

        // Some non-silent stores
        data[0] = i;     // Non-silent (writing different value)
        data[0] = 0;     // Silent again (writing 0 back)
    }

    // // End simulation region
    // m5_roi_end();

    printf("Test completed. Final values: %d %d %d %d\n",
           data[0], data[1], data[2], data[3]);

    return 0;
}