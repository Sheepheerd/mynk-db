#include <stdio.h>
#include "router.h"

Route routes[10];
int route_count = 0;

void register_route(void (*endpoint)()) {
    if (route_count >= 2) {
        return;
    }

    Route new_route = {endpoint};
    routes[route_count] = new_route;
    route_count++;
}

void route(method, url) {
    result = r.handler();

    return result;
}

Router router = {&router_logic};
