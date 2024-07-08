#ifndef MYQUEUE_H_
#define MYQUEUE_H_

struct node {
    int *client_socket;
    struct node *next;
};

typedef struct node node_t;

void enqueue(int *client_socket);
int *dequeue();

#endif