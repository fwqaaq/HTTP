#include "shared_memory.h"
#include <stdio.h>
#define BLOCK_SIZE 4096

int main(int argc, char **argv) {
    if (argc != 1) {
        printf("Usage: %s no args\n", argv[0]);
        return -1;
    }

    // attach to the shared memory block
    char *block = attach_memory_block(FILENAME, BLOCK_SIZE);
    if (block == NULL) {
        printf("Failed to attach to shared memory block\n");
        return -1;
    }

    printf("Read shared memory block:\n%s\n", block);
    detach_memory_block(block);

    return 0;
}