#ifndef EXAMPLE_ROUTES_H
#define EXAMPLE_ROUTES_H

#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>


char *get_hello(const char *data);

char *get_test(const char *data);

char *post_sync(const char *data);

#endif
