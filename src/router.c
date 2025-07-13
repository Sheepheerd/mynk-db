#include <stdio.h>
#include "router.h"

void hello_from_router() {
    printf("Hello From da routa\n");
}

void another_route() {
    printf("Hello From another_route\n");
}


// Route r1 = {&hello_from_router};
// Route r2 = {&another_route};
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
