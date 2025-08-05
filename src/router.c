#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "router.h"


static char *normalize_url(const char *url) {
    char *normalized = strdup(url);
    if (!normalized) return NULL;


    char *query = strchr(normalized, '?');
    if (query) *query = '\0';


    size_t len = strlen(normalized);
    if (len > 1 && normalized[len - 1] == '/') {
        normalized[len - 1] = '\0';
    }

    return normalized;
}

Router *router_init() {
    Router *router = malloc(sizeof(Router));
    if (!router) {
        fprintf(stderr, "Failed to allocate router\n");
        return NULL;
    }

    if (cc_hashtable_new(&router->routes) != CC_OK) {
        fprintf(stderr, "Failed to initialize routes hash table\n");
        free(router);
        return NULL;
    }

    return router;
}

void register_route(Router *router, const char *method, const char *url, char *(*handler)(const char *)) {
    if (!router || !router->routes || !method || !url || !handler) {
        fprintf(stderr, "Invalid arguments to register_route\n");
        return;
    }

    Route *new_route = malloc(sizeof(Route));
    if (!new_route) {
        fprintf(stderr, "Failed to allocate route\n");
        return;
    }

    char *normalized_url = normalize_url(url);
    if (!normalized_url) {
        free(new_route);
        fprintf(stderr, "Failed to normalize URL\n");
        return;
    }

    new_route->method = strdup(method);
    new_route->url = normalized_url; // Store normalized URL
    new_route->handler = handler;

    if (cc_hashtable_add(router->routes, normalized_url, new_route) != CC_OK) {
        fprintf(stderr, "Failed to add route %s:%s\n", method, normalized_url);
        free(new_route->method);
        free(new_route->url);
        free(new_route);
    } else {
        printf("Registered route %s:%s\n", method, normalized_url); // Debug log
    }
}

char *route(Router *router, const char *method, const char *url, const char *data) {
    if (!router || !router->routes || !method || !url) {
        fprintf(stderr, "Invalid arguments to route\n");
        return NULL;
    }

    char *normalized_url = normalize_url(url);
    if (!normalized_url) {
        fprintf(stderr, "Failed to normalize URL\n");
        return NULL;
    }

    printf("Looking up route %s:%s\n", method, normalized_url); // Debug log

    void *r = NULL;
    if (cc_hashtable_get(router->routes, normalized_url, &r) == CC_OK && r) {
        Route *route = (Route *)r;
        if (strcmp(route->method, method) == 0) {
            printf("Found route %s:%s\n", method, normalized_url); // Debug log
            char *result = route->handler(data);
            free(normalized_url);
            return result;
        }
    }

    fprintf(stderr, "Route not found for %s:%s\n", method, normalized_url);
    free(normalized_url);
    return NULL;
}

void router_free(Router *router) {
    if (!router || !router->routes) return;

    CC_HashTableIter iter;
    cc_hashtable_iter_init(&iter, router->routes);

    TableEntry *entry;
    while (cc_hashtable_iter_next(&iter, &entry) == CC_OK) {
        Route *route = (Route *)entry->value;
        free(route->method);
        free(route->url);
        free(route);
    }
    cc_hashtable_destroy(router->routes);
    free(router);
}

