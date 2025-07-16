#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *hello_from_router() {
    const char *filename = "./file_to_be_read_from"; // File path for goober in project root
    FILE *file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0'; // Null terminate the string

    fclose(file);
    return buffer;
}

char* another_route() {
    printf("Hello From another_route\n");
    return "";
}
