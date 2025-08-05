#include "example_routes.h"
#include <stdio.h>

char *another_route(const char *data) {
    (void)data;
    return strdup("Response from /test");
}

char *hello_from_router(const char *data) {
    (void)data;
    return strdup("Hello from router!");
}

char *parse_sync(const char *data) {
    if (!data) {
        return strdup("No POST data provided");
    }
    printf("this is the data: %s\n", data);
    return strdup("POST");
}
