#ifndef ROUTER_H
#define ROUTER_H

void hello_from_router();
void another_route();
void register_route(void (*endpoint)());

typedef struct Route {
  void (*handler)(void);
} Route;

extern Route routes[10];
#endif
