#ifndef ROUTER_H
#define ROUTER_H

typedef struct Router {
  void (*route)(void);
} Router;

extern Router router;

void hello_from_router();
void another_route();
void register_route(void (*endpoint)());
void router_logic();

typedef struct Route {
  void (*handler)(void);
} Route;

extern Route routes[10];
#endif
