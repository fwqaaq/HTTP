
#include "_queue.h"
#include <stdlib.h>

node_t *head = NULL;
node_t *tail = NULL;

// enqueue a new client socket
void enqueue(int *client_socket) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->client_socket = client_socket;
    new_node->next = NULL;
    if (tail == NULL) {
        head = new_node;
    } else {
        tail->next = new_node;
    }
    tail = new_node;
}

// dequeue the client socket at the head of the queue
int *dequeue() {
    if (head == NULL) {
        return NULL;
    }
    int *client_socket = head->client_socket;
    node_t *temp = head;
    head = head->next;
    if (head == NULL) {
        tail = NULL;
    }
    free(temp); // free the memory
    return client_socket;
}