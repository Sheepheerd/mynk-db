#ifndef ROUTER_H
#define ROUTER_H
#include "collectc/cc_hashtable.h"
#include <microhttpd.h>

typedef struct Router {
    char *(*route)(const char *, const char *);
} Router;

extern CC_HashTable *routes;
extern Router *router;

void register_route(char *(*endpoint)(), char *url, const char *method);
void router_init();
char *route(const char *method, char *url, char *post_body, struct MHD_Connection *connection);
char *response_string(char *response_string);

typedef struct Route {
    char *(*handler)(char *);
    enum MHD_Result (*iterator) (void *coninfo_cls,
                                 enum MHD_ValueKind kind,
                                 const char *key,
                                 const char *filename,
                                 const char *content_type,
                                 const char *transfer_encoding,
                                 const char *data,
                                 uint64_t off,
                                 size_t size);
} Route;

#endif
