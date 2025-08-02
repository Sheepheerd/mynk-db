#include <stddef.h>
#include <stdio.h>
#include "router.h"
#include "string.h"
#include <collectc/cc_hashtable.h>
#include <stdlib.h>



CC_HashTable *routes = NULL;
Router *router = NULL;

void register_route(char* (*endpoint)(), char *url) {
    Route *new_route = malloc(sizeof(Route));
    new_route->handler = endpoint;
    cc_hashtable_add(routes, url, new_route);
}

char *route(const char *method, const char *url) {
    void *r = NULL;
    cc_hashtable_get(routes, url, &r);
    Route *route = (Route *)r;
    // printf("this is what route->hanlder is returning: %s\n", route->handler());
    return route->handler();
}

void router_init() {
    router = malloc(sizeof(Router));
    if (cc_hashtable_new(&routes) != CC_OK) {
        printf("Bad thing happen");
    }
}


char *response_string(char *response_string) {
    size_t len = strlen(response_string) + 1;
    char *respCopy = malloc(len);
    memcpy(respCopy, response_string, len);
    return respCopy;

}
