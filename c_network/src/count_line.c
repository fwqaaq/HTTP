#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define SERVER_PORT 80
#define MAX_LINE 4096

void err_n_die(const char *fmt, ...);

int http_get(char *address, uint16_t port) {
    int fd;
    int send_bytes;
    struct sockaddr_in server_addr;
    char sendline[MAX_LINE];

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_n_die("Error while creating the socket");
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        err_n_die("inet_pton error for %s", address);
    }

    if (connect(fd, (const struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        err_n_die("connect failed");
    }

    double *b = calloc(sizeof(double), 100);
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");

    send_bytes = strlen(sendline);

    if (send(fd, sendline, send_bytes, 0) != send_bytes) {
        err_n_die("send error");
    }
    return fd;
}

int main(int argc, char **argv) {
    char *src = argv[1];
    FILE *f = NULL;

    if (strcmp(src, "stdin") == 0) {
        f = stdin;
    } else if (strcmp(src, "file") == 0) {
        f = fopen(argv[2], "r");
    } else if (strcmp(src, "http") == 0) {
        int socket = http_get(argv[2], SERVER_PORT);
        f = fdopen(socket, "r");
    } else {
        printf("error: invalid source: %s\n", src);
        return EXIT_FAILURE;
    }

    if (!f) {
        err_n_die("error while opening the file");
        return EXIT_FAILURE;
    }

    // read all the lines from source
    char buffer[MAX_LINE];
    int num_lines = 0;
    while (!feof(f)) {
        fgets(buffer, MAX_LINE, f);
        num_lines++;
    }

    fclose(f);
    printf("Number of lines: %d\n", num_lines);

    return EXIT_SUCCESS;
}

void err_n_die(const char *fmt, ...) {
    int errno_save;
    va_list ap;
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

    exit(EXIT_SUCCESS);
}