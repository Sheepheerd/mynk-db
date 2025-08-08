#include "routes.h"
#include "parser.h"

/*
* NOTES
*
* any returned strdup will be freed by the send_page method
*/


char *get_test(const char *data) {
    (void)data;
    return strdup("Response from /test\n");
}

char *get_hello(const char *data) {
    (void)data;
    return strdup("Hello from router!\n");
}

char *post_sync(const char *data) {
    if (!data) {
        return strdup("400");
    }
    parse_sync_post(data);

    return strdup(data);
}
