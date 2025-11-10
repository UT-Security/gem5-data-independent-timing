#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MATRIX_SIZE 100
#define ITERATIONS 50

// Matrix operations with many redundant/silent stores
int main() {
    printf("Starting matrix silent store benchmark...\n");

    // Allocate matrices
    int** matrix_a = (int**)malloc(MATRIX_SIZE * sizeof(int*));
    int** matrix_b = (int**)malloc(MATRIX_SIZE * sizeof(int*));
    int** matrix_c = (int**)malloc(MATRIX_SIZE * sizeof(int*));
    int** matrix_temp = (int**)malloc(MATRIX_SIZE * sizeof(int*));

    for (int i = 0; i < MATRIX_SIZE; i++) {
        matrix_a[i] = (int*)malloc(MATRIX_SIZE * sizeof(int));
        matrix_b[i] = (int*)malloc(MATRIX_SIZE * sizeof(int));
        matrix_c[i] = (int*)malloc(MATRIX_SIZE * sizeof(int));
        matrix_temp[i] = (int*)malloc(MATRIX_SIZE * sizeof(int));
    }

    // Initialize matrices
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) % 10;
            matrix_b[i][j] = (i - j + 10) % 10;
            matrix_c[i][j] = 0;
            matrix_temp[i][j] = 0;
        }
    }

    // Main benchmark loop
    for (int iter = 0; iter < ITERATIONS; iter++) {

        // Operation 1: Diagonal update (mostly silent)
        for (int i = 0; i < MATRIX_SIZE; i++) {
            matrix_a[i][i] = matrix_a[i][i];  // 100% silent on diagonal
            if (i > 0) {
                matrix_a[i][i-1] = matrix_a[i][i-1];  // 100% silent
            }
        }

        // Operation 2: Matrix element-wise maximum (many silent stores)
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                int max_val = (matrix_a[i][j] > matrix_b[i][j]) ?
                              matrix_a[i][j] : matrix_b[i][j];
                matrix_c[i][j] = max_val;  // Silent when max doesn't change
            }
        }

        // Operation 3: Transpose with redundancy
        if (iter % 2 == 0) {
            // Copy to temp
            for (int i = 0; i < MATRIX_SIZE; i++) {
                for (int j = 0; j < MATRIX_SIZE; j++) {
                    matrix_temp[i][j] = matrix_a[i][j];
                }
            }
        } else {
            // Write temp to itself (100% silent)
            for (int i = 0; i < MATRIX_SIZE; i++) {
                for (int j = 0; j < MATRIX_SIZE; j++) {
                    matrix_temp[i][j] = matrix_temp[i][j];  // 100% silent
                }
            }
        }

        // Operation 4: Boundary updates (high silent rate)
        for (int i = 0; i < MATRIX_SIZE; i++) {
            // Top and bottom rows
            matrix_b[0][i] = 1;  // Silent after first iteration
            matrix_b[MATRIX_SIZE-1][i] = 1;  // Silent after first iteration

            // Left and right columns
            matrix_b[i][0] = 2;  // Silent after first iteration
            matrix_b[i][MATRIX_SIZE-1] = 2;  // Silent after first iteration
        }

        // Operation 5: Sparse update pattern
        for (int i = 0; i < MATRIX_SIZE; i += 8) {
            for (int j = 0; j < MATRIX_SIZE; j += 8) {
                // Update small blocks, many will be silent
                for (int bi = 0; bi < 3 && i+bi < MATRIX_SIZE; bi++) {
                    for (int bj = 0; bj < 3 && j+bj < MATRIX_SIZE; bj++) {
                        int old_val = matrix_c[i+bi][j+bj];
                        matrix_c[i+bi][j+bj] = old_val % 5;  // Often silent
                    }
                }
            }
        }
    }

    // Calculate checksum
    int checksum = 0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            checksum += matrix_a[i][j] + matrix_b[i][j] +
                       matrix_c[i][j] + matrix_temp[i][j];
        }
    }

    printf("Benchmark completed. Checksum: %d\n", checksum);

    // Free memory
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrix_a[i]);
        free(matrix_b[i]);
        free(matrix_c[i]);
        free(matrix_temp[i]);
    }
    free(matrix_a);
    free(matrix_b);
    free(matrix_c);
    free(matrix_temp);

    return 0;
}