#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define LIST_SIZE 1000
#define ITERATIONS 500

typedef struct Node {
    int value;
    struct Node* next;
    int visited;
    int metadata;
} Node;

// Linked list benchmark with pointer chasing and redundant metadata updates
int main() {
    printf("Starting linked list silent store benchmark...\n");

    // Create linked list
    Node* head = (Node*)malloc(sizeof(Node));
    head->value = 0;
    head->visited = 0;
    head->metadata = 0;
    head->next = NULL;

    Node* current = head;
    for (int i = 1; i < LIST_SIZE; i++) {
        Node* new_node = (Node*)malloc(sizeof(Node));
        new_node->value = i;
        new_node->visited = 0;
        new_node->metadata = i % 10;
        new_node->next = NULL;
        current->next = new_node;
        current = new_node;
    }

    // Benchmark: Multiple passes with different patterns
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Pass 1: Mark all as visited (silent after first iteration)
        current = head;
        while (current != NULL) {
            current->visited = 1;  // Silent store after first iteration
            current = current->next;
        }

        // Pass 2: Update metadata conditionally
        current = head;
        int counter = 0;
        while (current != NULL) {
            int old_metadata = current->metadata;

            // Complex condition that often results in same value
            if (counter % 3 == 0) {
                current->metadata = old_metadata;  // 33% silent stores
            } else if (counter % 3 == 1) {
                current->metadata = (old_metadata + 1) % 10;
            } else {
                current->metadata = old_metadata;  // Another 33% silent
            }

            counter++;
            current = current->next;
        }

        // Pass 3: Redundant pointer updates (100% silent)
        current = head;
        while (current != NULL && current->next != NULL) {
            Node* next_temp = current->next;
            current->next = next_temp;  // 100% silent store
            current = next_temp;
        }

        // Pass 4: Value updates with high silent rate
        current = head;
        while (current != NULL) {
            int val = current->value;

            // Operation that often produces same value
            if (val % 2 == 0) {
                current->value = val;  // 50% silent stores
            } else {
                current->value = val + 1 - 1;  // Also silent!
            }

            current = current->next;
        }
    }

    // Calculate checksum
    int checksum = 0;
    current = head;
    while (current != NULL) {
        checksum += current->value + current->visited + current->metadata;
        current = current->next;
    }

    printf("Benchmark completed. Checksum: %d\n", checksum);

    // Free memory
    current = head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}