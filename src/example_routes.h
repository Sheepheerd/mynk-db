#ifndef EXAMPLE_ROUTES_H
#define EXAMPLE_ROUTES_H

#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>


char *another_route(const char *data);

char *hello_from_router(const char *data);

char *parse_sync(const char *data);

#endif
