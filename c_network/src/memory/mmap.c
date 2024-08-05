#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define PAGESIZE 4096

int v = 5;

int main(int argc, char **argv) {
    /**
     * @param addr: The starting address for the new mapping. If addr is NULL,
     * then the kernel chooses the address at which to create the mapping; this
     * is the most portable method of creating a new mapping.
     * @param length: The length of the mapping.
     * @param prot: The desired memory protection of the mapping.
     * common values are: PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE
     * @param flags: The type of the mapping.
     * common values are: MAP_SHARED, MAP_PRIVATE, MAP_ANONYMOUS
     * @param fd: The file descriptor of the file which is mapped.
     * if MAP_ANONYMOUS is set, fd is ignored and should be -1.
     * @param offset: The offset in the file where the mapping starts.
     * must be a multiple of the page size (usually 4096).
     */
    uint8_t *memory_shared = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *memory_shared = 5;
    if (fork() == 0) {
        *memory_shared = 10;
        v = 80;
    } else {
        wait(NULL);
    }

    printf("Not Shared. v = %d\n", v);
    printf("Shared. *memory_shared = %d\n", *memory_shared);
}