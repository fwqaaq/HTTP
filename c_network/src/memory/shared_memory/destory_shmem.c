
#include "shared_memory.h"
#include <stdio.h>
int main(int argc, char **argv) {
    if (argc != 1) {
        printf("Usage: %s no args\n", argv[0]);
        return -1;
    }

    if (destroy_memory_block(FILENAME)) {
        printf("Shared memory block destroyed\n");
    } else {
        printf("Failed to destroy shared memory block\n");
    }

    return 0;
}