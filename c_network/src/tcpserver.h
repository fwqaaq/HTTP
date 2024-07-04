
#include <stddef.h>

#define SERVER_PORT 18000
#define MAXLINE 4096

void err_n_die(const char *fmt, ...);
char *bin2hex(const unsigned char *input, size_t len);
