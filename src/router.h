#ifndef ROUTER_H
#define ROUTER_H
#include "collectc/cc_hashtable.h"

typedef struct Router {
    char *(*route)(const char *, const char *);
} Router;

extern CC_HashTable *routes;
extern Router *router;

void register_route(char *(*endpoint)(), char *url);
void router_init();
char *route(const char *method, const char *url);
char *response_string(char *response_string);

typedef struct Route {
    char *(*handler)(void);
} Route;

#endif
