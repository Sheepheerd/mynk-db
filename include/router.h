#ifndef ROUTER_H
#define ROUTER_H

#include <collectc/cc_hashtable.h>

typedef struct {
    char *(*handler)(const char *data);
    char *url;
    char *method;
} Route;

typedef struct {
    CC_HashTable *routes;
} Router;

Router *router_init();
void register_route(Router *router, const char *method, const char *url, char *(*handler)(const char *));
char *route(Router *router, const char *method, const char *url, const char *data);
void router_free(Router *router);

#endif
