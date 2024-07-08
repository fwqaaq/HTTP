#include "tcpserver.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/_endian.h>
#include <sys/_types/_socklen_t.h>
#include <sys/_types/_va_list.h>
#include <sys/socket.h>
#include <unistd.h>

int create_tcp_server() {
    int fd;
    struct sockaddr_in server_addr;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_n_die("socket error.");
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    // INADDR_ANY(0.0.0.0) allows the server to accept a client connection on
    // any interface
    // htonl -> host to network long
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // htons -> host to network short
    server_addr.sin_port = htons(SERVER_PORT);

    // bind the socket
    if ((bind(fd, (const struct sockaddr *)&server_addr,
              sizeof(server_addr))) == -1) {
        err_n_die("bind error.");
        close(fd);
        exit(1);
    }

    // param 1: fd is the socket file descriptor
    // param 2: backlog is the maximum number of pending connections that
    // can be
    if (listen(fd, 10) == -1) {
        err_n_die("listen error.");
        close(fd);
        exit(1);
    }

    printf("waiting for a connection on port %d\n", SERVER_PORT);
    fflush(stdout);

    return fd;
}

int acceptClient(int fd) {
    int conn;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        char client_address[MAXLINE + 1];

        conn = accept(fd, (struct sockaddr *)&client_addr, &client_addr_len);

        // https://www.cnblogs.com/fortunely/p/14916296.html#21-inet_aton%E5%87%BD%E6%95%B0
        inet_ntop(AF_INET, &client_addr, client_address, MAXLINE);
        printf("accept a connection from %s\n", client_address);

        if (conn < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_n_die("accept error.");
                return -1;
            }
        }
        break;
    }

    return conn;
}

int main(int argc, char **argv) {
    uint8_t buffer[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];

    int server_socket = create_tcp_server();

    if (server_socket < 0) {
        err_n_die("create_tcp_server error.");
    }

    while (1) {
        int client_scoket = acceptClient(server_socket);
        int n;
        while ((n = read(client_scoket, recvline, MAXLINE - 1)) > 0) {
            fprintf(stdout, "\n%s\n\n%s", bin2hex(recvline, n), recvline);

            if (recvline[n - 1] == '\n')
                break;
            memset(recvline, 0, MAXLINE);
        }

        if (n < 0) {
            err_n_die("read error");
        }
        snprintf((char *)buffer, sizeof(buffer),
                 "HTTP/1.0 200 OK\r\n\r\nHello");
        write(client_scoket, (char *)buffer, strlen((char *)buffer));
        close(client_scoket);
    }
    close(server_socket);
}

void err_n_die(const char *fmt, ...) {
    va_list ap;
    int errno_save;

    errno_save = errno;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    if (errno_save != 0) {
        fprintf(stdout, "(errno = %d) : %s\n", errno_save,
                strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    va_end(ap);

    exit(0);
}

char *bin2hex(const unsigned char *input, size_t len) {
    char *result;
    char *hexits = "0123456789ABCDEF";

    if (input == NULL || len <= 0) {
        return NULL;
    }

    int result_length = (len * 3) + 1;
    result = malloc(result_length);
    bzero(result, result_length);

    for (int i = 0; i < len; i++) {
        result[i * 3] = hexits[input[i] >> 4];
        result[(i * 3) + 1] = hexits[input[i] & 0x0F];
        result[(i * 3) + 2] = ' ';
    }
    return result;
}