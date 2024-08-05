

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
uint64_t get_mem_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

int main(int argc, char **argv) {
    uint64_t baseline = get_mem_usage();
    for (int i = 0; i < 1000; i++) {
        char *p = malloc(1024 * 100);
        memset(p, 1, 1024 * 100);
        printf("Memory usage: %llu\n", get_mem_usage() - baseline);
    }
    return 0;
}