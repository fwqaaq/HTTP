
#include "lib/_queue.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_pthread/_pthread_cond_t.h>
#include <sys/_pthread/_pthread_mutex_t.h>
#include <sys/_pthread/_pthread_t.h>
#include <sys/_types/_socklen_t.h>
#include <sys/socket.h>
#include <sys/syslimits.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096
#define SOCKET_ERROR (-1)
#define SERVER_BACKLOG 100
#define THREAD_POOL_SIZE 10

void *handle_connection(void *p_client_socket);
int check(int exp, const char *msg);
void *thread_function(void *arg);

pthread_t threads[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char **argv) {
    int server_socket, client_socket;
    socklen_t addr_size;
    struct sockaddr_in server_addr, client_addr;
    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)),
          "Failed to create socket");

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        /**
        @param t: thread id
        @param NULL: default thread attributes
        @param handle_connection: function to be executed by the thread
        @param p_client: argument to the function
        */
        pthread_create(&threads[i], NULL, thread_function, NULL);
    }

    // initialize the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    check(bind(server_socket, (const struct sockaddr *)&server_addr,
               sizeof(server_addr)),
          "Bind failed");
    check(listen(server_socket, SERVER_BACKLOG), "Listen failed");

    while (true) {
        printf("Waiting for connections...\n");
        // wait for, and eventually accept an incoming connection
        addr_size = sizeof(struct sockaddr_in);
        check(client_socket = accept(
                  server_socket, (struct sockaddr *)&client_addr, &addr_size),
              "accept failed");

        int *p_client = malloc(sizeof(int));
        *p_client = client_socket;
        pthread_mutex_lock(&mutex);
        // signal a thread to pick up the connection
        pthread_cond_signal(&cond);
        enqueue(p_client);
        pthread_mutex_unlock(&mutex);
        // pthread_t t;
        // pthread_create(&t, NULL, handle_connection, p_client);
        // handle_connection(p_client);
    }
    return 0;
}

int check(int exp, const char *msg) {
    if (exp == SOCKET_ERROR) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void *thread_function(void *arg) {
    while (1) {
        int *p_client_socket;
        pthread_mutex_lock(&mutex);

        // if the queue is empty, wait for a signal
        // Because pthread_cond_signal may be called too many times,
        // cause blocking thread
        if ((p_client_socket = dequeue()) == NULL) {
            pthread_cond_wait(&cond, &mutex); // wait for a signal (enqueue)
            // try again
            p_client_socket = dequeue();
        }
        pthread_mutex_unlock(&mutex);
        if (p_client_socket != NULL) {
            handle_connection(p_client_socket);
        }
    }
}

void *handle_connection(void *p_client_socket) {
    int client_socket = *((int *)p_client_socket);
    free(p_client_socket); // free the memory allocated in main

    char buffer[BUFFER_SIZE];
    // size_t is an unsigned integer type, for which sizeof, malloc etc. return
    // depending on the system architecture (32/64 bit)
    size_t bytes_read;
    int msg_size = 0;
    char actual_path[PATH_MAX + 1];

    // read the message from the client
    while ((bytes_read = read(client_socket, buffer + msg_size,
                              sizeof(buffer) - msg_size - 1)) > 0) {
        msg_size += bytes_read; // update the message size
        if (msg_size > BUFFER_SIZE - 1 || buffer[msg_size - 1] == '\n') {
            break;
        }
    }

    check(bytes_read, "recv failed");
    buffer[msg_size - 1] = 0; // null-terminate the message and remove the \n

    printf("Received message: %s\n", buffer);
    fflush(stdout);

    // validate the path
    if (realpath(buffer, actual_path) == NULL) {
        printf("ERROR(bad path): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    FILE *fd = fopen(actual_path, "r");
    if (fd == NULL) {
        printf("ERROR(open): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    // read the file and send it to the client
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fd)) > 0) {
        printf("Sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }
    close(client_socket);
    fclose(fd);
    printf("Closing connection\n");
    return NULL;
}