#ifndef ROUTER_H
#define ROUTER_H
#include "collectc/cc_hashtable.h"

typedef struct Router {
  int (*route)(const char *, const char *);
} Router;

extern CC_HashTable *routes;
extern Router *router;

void register_route(void (*endpoint)(), const char *url);
void router_init();
int route(const char *method, const char *url);

typedef struct Route {
  void (*handler)(void);
} Route;

#endif
