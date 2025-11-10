#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1000
#define ITERATIONS 100

// Swap function that often results in silent stores
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Conditional swap - many will be silent
void conditional_swap(int* a, int* b) {
    if (*a > *b) {
        swap(a, b);
    } else {
        // Write same values back (silent stores)
        *a = *a;  // Silent store
        *b = *b;  // Silent store
    }
}

// Bubble sort variant with extra writes
void bubble_sort_with_redundancy(int arr[], int n) {
    for (int i = 0; i < n-1; i++) {
        int swapped = 0;
        for (int j = 0; j < n-i-1; j++) {
            // Always write, even if no swap needed
            if (arr[j] > arr[j+1]) {
                swap(&arr[j], &arr[j+1]);
                swapped = 1;
            } else {
                // Redundant writes (silent stores)
                arr[j] = arr[j];      // Silent store
                arr[j+1] = arr[j+1];  // Silent store
            }
        }
        // If no swaps, array is sorted - all future passes will be silent
        if (!swapped) {
            // Continue anyway to generate silent stores
            for (int k = i+1; k < n-1; k++) {
                for (int j = 0; j < n-k-1; j++) {
                    arr[j] = arr[j];      // 100% silent stores
                    arr[j+1] = arr[j+1];  // 100% silent stores
                }
            }
            break;
        }
    }
}

int main() {
    printf("Starting sorting silent store benchmark...\n");

    // Allocate arrays
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* backup = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* sorted = (int*)malloc(ARRAY_SIZE * sizeof(int));

    // Initialize with partially sorted data (more silent stores)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (i < ARRAY_SIZE/2) {
            data[i] = i;  // First half already sorted
        } else {
            data[i] = rand() % 100;  // Second half random
        }
        backup[i] = data[i];
        sorted[i] = i;  // Fully sorted reference
    }

    // Main benchmark loop
    for (int iter = 0; iter < ITERATIONS; iter++) {

        // Phase 1: Sort the array (many passes will be silent)
        memcpy(data, backup, ARRAY_SIZE * sizeof(int));
        bubble_sort_with_redundancy(data, ARRAY_SIZE);

        // Phase 2: Re-sort already sorted array (100% silent stores)
        bubble_sort_with_redundancy(data, ARRAY_SIZE);

        // Phase 3: Merge-like operation with silent stores
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (data[i] == sorted[i]) {
                data[i] = sorted[i];  // Silent store
                sorted[i] = sorted[i];  // Silent store
            } else {
                data[i] = sorted[i];  // Non-silent
            }
        }

        // Phase 4: Partition-like operation
        int pivot = 50;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int val = data[i];
            if (val < pivot) {
                data[i] = val;  // Silent store
            } else if (val > pivot) {
                data[i] = val;  // Silent store
            } else {
                data[i] = pivot;  // Might be silent
            }
        }

        // Phase 5: Find min/max with redundant writes
        int min_val = data[0];
        int max_val = data[0];
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (data[i] < min_val) {
                min_val = data[i];
            } else {
                min_val = min_val;  // Silent assignment
            }

            if (data[i] > max_val) {
                max_val = data[i];
            } else {
                max_val = max_val;  // Silent assignment
            }

            // Write back min/max to special positions (often silent)
            data[0] = min_val;  // Often silent after first few iterations
            data[ARRAY_SIZE-1] = max_val;  // Often silent
        }
    }

    // Calculate checksum
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += data[i] + sorted[i];
    }

    printf("Benchmark completed. Checksum: %d\n", checksum);

    free(data);
    free(backup);
    free(sorted);

    return 0;
}