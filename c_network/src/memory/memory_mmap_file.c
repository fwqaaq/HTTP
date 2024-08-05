#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <unistd.h>

int main(int argc, char **argv) {

    char resolved_path[PATH_MAX];
    if (realpath("./clang", resolved_path) != NULL) {
        printf("Resolved path: %s\n", resolved_path);
    } else {
        perror("Not able to resolve path");
    }
    // and mmap can be used to map a file into memory
    /**
     * @param file: the file path
     * @param flags: the flags for opening the file
     * @param mode: the mode for opening the file
     */
    int fd = open(resolved_path, O_RDWR, S_IRUSR | S_IWUSR);
    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror("could not get file size");
    }
    printf("file size: %lld\n", st.st_size);

    char *file_in_memory =
        mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    printf("Printing file, as an array of characters...");
    for (int i = 0; i < st.st_size; i++) {
        // printf("%c", file_in_memory[i]);
        if ((i % 2) == 0) {
            file_in_memory[i] = 'X';
        }
        printf("%c", file_in_memory[i]);
    }
    printf("\n");

    // free the starting address of the mapping for mmap
    munmap(file_in_memory, st.st_size);
    close(fd);
}