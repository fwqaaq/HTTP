
#include "shared_memory.h"
#include <stdio.h>
#include <string.h>
#define BLOCK_SIZE 4096

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s [stuff to write]\n", argv[0]);
        return -1;
    }

    // grab the shared memory block
    char *block = attach_memory_block(FILENAME, BLOCK_SIZE);
    if (block == NULL) {
        printf("Failed to attach shared memory block\n");
        return -1;
    }

    printf("Writing to shared memory: %s\n", argv[1]);
    strncpy(block, argv[1], BLOCK_SIZE);

    detach_memory_block(block);
    return 0;
}