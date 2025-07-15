#include <stdio.h>
#include "router.h"
#include <collectc/cc_hashtable.h>



CC_HashTable *routes = NULL;
Router *router = NULL;

void register_route(char* (*endpoint)(), const char *url) {
    Route *new_route = malloc(sizeof(Route));
    new_route->handler = endpoint;
    cc_hashtable_add(routes, url, new_route);
}

char* route(const char *method, const char *url) {
    void *r = NULL;
    cc_hashtable_get(routes, url, &r);
    Route *route = (Route *)r;
    // char* result = route->handler();
    char *bundle_of_sticks = "joe hogan";
    // printf("%zu\n", strlen(result));
    return bundle_of_sticks;
}

void router_init() {
    router = malloc(sizeof(Router));
    if (cc_hashtable_new(&routes) != CC_OK) {
        printf("Bad thing happen");
    }
}
