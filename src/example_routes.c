#include <stdio.h>
#include <string.h>
#include "router.h"

char* hello_from_router() {

    size_t bufsize = 128;       // initial buffer size
    char *buffer = malloc(bufsize);
    if (!buffer) return NULL;   // check allocation

    size_t pos = 0;
    int c;


    while ((c = getchar()) != '\n' && c != EOF) {
        buffer[pos++] = (char)c;

        // Resize if needed
        if (pos >= bufsize) {
            bufsize *= 2;
            char *new_buffer = realloc(buffer, bufsize);
            if (!new_buffer) {
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
    }
    buffer[pos] = '\0';  // null-terminate string
    return buffer;



}

char* another_route() {
    printf("Hello From another_route\n");
}
