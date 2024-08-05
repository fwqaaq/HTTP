

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096
#define BUFFER_OFFSET 10

typedef struct node {
    uint32_t is_vaild : 1;
    uint32_t size : 15;
    struct node *next;
} node_t;

int main(int argc, char **argv) {
    node_t n1 = {.is_vaild = 1, .size = 10, .next = NULL};
    node_t *n2;

    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);
    memcpy(buffer + BUFFER_OFFSET, &n1, sizeof(node_t));
    n2 = (node_t *)(buffer + BUFFER_OFFSET);

    printf("is_vaild: %d, size: %d, next: %p\n", n2->is_vaild, n2->size,
           n2->next);
}