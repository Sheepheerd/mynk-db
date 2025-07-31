#include <stddef.h>
#include <stdio.h>
#include "router.h"
#include "string.h"
#include <collectc/cc_hashtable.h>
#include <stdlib.h>
#include <microhttpd.h>


CC_HashTable *post_routes = NULL;
CC_HashTable *get_routes = NULL;

Router *router = NULL;



struct connection_info_struct
{
    int connectiontype;
    char *answerstring;
    struct MHD_PostProcessor *postprocessor;
};



void register_route(char* (*endpoint)(), char *url, const char *method) {

    Route *new_route = malloc(sizeof(Route));
    new_route->handler = endpoint;
    cc_hashtable_add(routes, url, new_route);
}

char *route(const char *method, char *url, char* post_body, struct MHD_Connection *connection) {
    struct connection_info_struct *con_info;

    con_info = malloc (sizeof (struct connection_info_struct));
    if (NULL == con_info)
        return MHD_NO;
    con_info->answerstring = NULL;

    void *r = NULL;

    cc_hashtable_get(post_body != NULL ? get_routes : post_routes, url, &r);

    Route *route = (Route *)r;
    if (post_body != NULL)
    {
        //
    } else {
        con_info->postprocessor =
            MHD_create_post_processor (connection, 10,
                                       route->iterator, (void *) con_info);
    }

    // printf("this is what route->hanlder is returning: %s\n", route->handler());
    return route->handler(post_body);
}

void router_init() {
    router = malloc(sizeof(Router));
    if (cc_hashtable_new(&routes) != CC_OK) {
        printf("Bad thing happen");
    }
}


char *response_string(char *response_string) {
    size_t len = strlen(response_string) + 1;
    char *respCopy = malloc(len);
    memcpy(respCopy, response_string, len);
    return respCopy;

}
