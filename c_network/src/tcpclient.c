#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 80
#define MAXLINE 4096

void err_n_die(const char *fmt, ...);

int main(int argc, char **argv) {
    int sockfd, n;
    int sendbytes;
    // ipv6 -> sockaddr_in6
    // ipv4 address structure
    struct sockaddr_in server_addr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    if (argc != 2) {
        err_n_die("usage: %s <server address>", argv[0]);
    }

    // param 1(domain): address family, for example AF_INET(IPv4),
    // AF_INET6(IPv6), AF_UNIX etc. param 2(type): socket type, for example
    // SOCK_STREAM(TCP), SOCK_DGRAM(UDP), SOCK_RAW(like ICMP, directly access to
    // network layer) etc. param 3(special protocol): usually set to 0(default
    // value), for example type is SOCK_STREAM, protocol is 0, then it will use
    // TCP, if type is SOCK_DGRAM, then UDP.
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_n_die("Error while creating the socket");
    }

    // zero out the server_addr struct (for network protocol, we need to zero
    // out)
    // memset(&server_addr, 0, sizeof(server_addr));
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // host to network short
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        err_n_die("inet_pton error for %s", argv[1]);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        err_n_die("connect failed");
    }

    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
    sendbytes = strlen(sendline);

    // send the request to the server
    if (write(sockfd, sendline, sendbytes) != sendbytes) {
        err_n_die("write error");
    }

    // set buffer to 0
    memset(recvline, 0, MAXLINE);

    // Update recvline buffer,
    while ((n = read(sockfd, recvline, MAXLINE - 2)) > 0) {
        // set the last byte to null, prevent new data mixing with old data
        recvline[n] = '\0';
        printf("%s", recvline);
    }

    if (n < 0) {
        err_n_die("read error");
    }

    exit(0);
}

/// fmt is the format string, like printf
/// https://welts.xyz/2021/04/10/varargs/
void err_n_die(const char *fmt, ...) {
    int errno_save;
    va_list ap; // declare a va_list type variable

    // any system or library call can set errno, so we need to save it
    errno_save = errno;

    // print out the fmt+args to standard out
    va_start(ap, fmt); // initialize the va_list(...)
    // for loop + va_arg to print out the arguments: va_arg(ap, char *);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    // print out error message if errno was set
    if (errno_save != 0) {
        fprintf(stdout, "(errno = %d) : %s\n", errno_save,
                strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    va_end(ap);

    exit(1);
}