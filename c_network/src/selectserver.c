
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/syslimits.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define SOCKET_ERROR (-1)
#define SERVER_PORT 8080
#define SERVER_BACKLOG 100

void *handle_connection(int client_socket);
int check(int exp, const char *msg);
int accept_client(int server_socket);
int create_tcp_server(__uint16_t port, int max_clients);

int main(int argc, char **argv) {
    int server_socket = create_tcp_server(SERVER_PORT, SERVER_BACKLOG);
    int max_socket_connection = 0;

    // create a set of file descriptors
    fd_set current_sockets, ready_sockets;

    // initialize current set when using select fucntion
    // FD_ZERO clears the set making it contain no file descriptors
    FD_ZERO(&current_sockets);
    // FD_SET adds a file descriptor to the set
    FD_SET(server_socket, &current_sockets);

    max_socket_connection = server_socket;

    printf("max socket connection %d\n", max_socket_connection);

    while (true) {
        // because select is destructive, we need to copy the set
        ready_sockets = current_sockets;
        // 1st parameter is the maximum file descriptor value
        // 2nd paramster is the set of file descriptors to be monitored for
        // read(monitoring stdout, whether socket data is available to read
        // etc.)
        // 3rd parameter is the set of file descriptors to be monitored for
        // write(monitoring stdin, whether data can be written to the socket
        // etc.)
        // 4th parameter is the set of file descriptors to be monitored for
        // exceptions (monitoring for exceptions such as out-of-band data)
        // 5th parameter is the maximum time to wait for an event to occur
        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        // FD_SETSIZE: maximum number of file descriptors in a set
        for (int i = 0; i <= max_socket_connection; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == server_socket) {
                    // this is a new connection
                    int client_socket = accept_client(server_socket);
                    FD_SET(client_socket, &current_sockets);
                    if (client_socket > max_socket_connection) {
                        max_socket_connection = client_socket;
                    }
                } else {
                    // this is an existing connection
                    handle_connection(i);
                    // remove the socket from the set
                    FD_CLR(i, &current_sockets);
                }
            }
        }
    }
}

int create_tcp_server(__uint16_t port, int backlog) {
    int server_socket, client_socket, addr_size;
    struct sockaddr_in server_addr;

    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)),
          "Failed to create socket");

    // initialize the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    check(bind(server_socket, (struct sockaddr *)&server_addr,
               sizeof(server_addr)),
          "Failed to bind");
    check(listen(server_socket, backlog), "Failed to listen");
    return server_socket;
}

int accept_client(int server_socket) {
    int addr_size = sizeof(struct sockaddr_in);
    int client_socket;
    struct sockaddr_in client_addr;
    check(
        (client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
                                (socklen_t *)&addr_size)),
        "Failed to accept");
    return client_socket;
}

void *handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    int msg_size = 0;
    char actual_path[PATH_MAX + 1];

    while ((bytes_read = read(client_socket, buffer + msg_size,
                              sizeof(buffer) - msg_size - 1)) > 0) {
        msg_size += bytes_read;
        if (msg_size > BUFFER_SIZE || buffer[msg_size - 1] == '\n') {
            break;
        }
    }

    check(bytes_read, "Failed to read");
    buffer[msg_size - 1] = '\0';

    printf("Received: %s\n", buffer);
    fflush(stdout);

    // validity check
    if (realpath(buffer, actual_path) == NULL) {
        printf("ERROR(bad path): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    // open the file
    FILE *fd = fopen(actual_path, "r");

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fd)) > 0) {
        write(client_socket, buffer, bytes_read);
    }

    close(client_socket);
    fclose(fd);
    printf("Closing connection\n");
    return NULL;
}

int check(int exp, const char *msg) {
    if (exp == SOCKET_ERROR) {
        perror(msg);
        exit(EXIT_FAILURE);
    }

    return exp;
}