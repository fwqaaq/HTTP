#include <curl/curl.h>
#include <curl/easy.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

size_t get_data(char *buffer, size_t time_size, size_t n_times,
                void *ignore_this) {
    size_t bytes = time_size * n_times;
    int line_number = 1;
    printf("New chunk: (%zu bytes)\n", bytes);
    printf("%d:\t", line_number);

    for (int i = 0; i < bytes; i++) {
        // print the character
        printf("%c", buffer[i]);
        // next line
        if (buffer[i] == '\n') {
            line_number++;
            printf("%d:\t", line_number);
        }
    }

    printf("\n\n");
    return bytes;
}

int main(int argc, char **argv) {
    CURL *curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "init failed\n");
        return EXIT_FAILURE;
    }

    // set request URL
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.bing.com/");
    // set callback function to process the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_data);

    // perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "download failed: %s\n", curl_easy_strerror(res));
        return EXIT_FAILURE;
    }

    curl_easy_cleanup(curl);

    return EXIT_SUCCESS;
}