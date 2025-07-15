#ifndef ROUTER_H
#define ROUTER_H
#include "collectc/cc_hashtable.h"

typedef struct Router {
  int (*route)(const char *, const char *);
} Router;

extern CC_HashTable *routes;

void register_route(void (*endpoint)(), const char *url);
void router_init();

typedef struct Route {
  void (*handler)(void);
} Route;

#endif
