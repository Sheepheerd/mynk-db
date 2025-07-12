#ifndef ROUTER_H
#define ROUTER_H

void hello_from_router();
void another_route();

typedef struct Route {
  void (*handler)(void);
} Route;
#endif
