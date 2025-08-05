#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "router.h"
#include "example_routes.h"

#if defined(_MSC_VER) && _MSC_VER <= 1800
#endif

#define PORT 8888
#define POSTBUFFERSIZE 512
#define MAXNAMESIZE 20
#define MAXANSWERSIZE 512

#define GET "GET"
#define POST "POST"

struct connection_info_struct {
    char *connectiontype;
    char *answerstring;
    size_t data_size;
};

static enum MHD_Result send_page(struct MHD_Connection *connection, char *page) {
    if (!page) {
        page = strdup("Internal Server Error");
    }

    struct MHD_Response *response = MHD_create_response_from_buffer(
                                        strlen(page), page, MHD_RESPMEM_MUST_FREE);
    if (!response) {
        free(page);
        return MHD_NO;
    }

    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

static void request_completed(void *cls, struct MHD_Connection *connection,
                              void **con_cls, enum MHD_RequestTerminationCode toe) {
    (void)cls;
    (void)connection;
    (void)toe;

    struct connection_info_struct *con_info = *con_cls;
    if (!con_info) return;

    if (strcmp(con_info->connectiontype, POST) == 0 && con_info->answerstring) {
        free(con_info->answerstring);
    }

    free(con_info->connectiontype);
    free(con_info);
    *con_cls = NULL;
}

static enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection,
        const char *url, const char *method,
        const char *version, const char *upload_data,
        size_t *upload_data_size, void **con_cls) {
    (void)version;
    Router *router = (Router *)cls;

    if (!*con_cls) {
        struct connection_info_struct *con_info = malloc(sizeof(struct connection_info_struct));
        if (!con_info) return MHD_NO;
        con_info->answerstring = NULL;
        con_info->data_size = 0;
        con_info->connectiontype = strdup(strcmp(method, POST) == 0 ? POST : GET);
        if (!con_info->connectiontype) {
            free(con_info);
            return MHD_NO;
        }
        *con_cls = (void *)con_info;
        return MHD_YES;
    }

    if (strcmp(method, GET) == 0) {
        // THIS IS WHERE YOU HANDLE GET DATA
        char *result = route(router, method, url, NULL);
        if (!result) {
            return send_page(connection, strdup("404 Not Found"));
        }
        // TODO
        return send_page(connection, result);
    }

    if (strcmp(method, POST) == 0) {
        struct connection_info_struct *con_info = *con_cls;

        if (*upload_data_size != 0) {
            if (!con_info->answerstring) {
                con_info->answerstring = malloc(*upload_data_size + 1);
                if (!con_info->answerstring) return MHD_NO;
                con_info->answerstring[0] = '\0';
                con_info->data_size = 0;
            } else {
                char *temp = realloc(con_info->answerstring, con_info->data_size + *upload_data_size + 1);
                if (!temp) {
                    free(con_info->answerstring);
                    con_info->answerstring = NULL;
                    return MHD_NO;
                }
                con_info->answerstring = temp;
            }

            memcpy(con_info->answerstring + con_info->data_size, upload_data, *upload_data_size);
            con_info->data_size += *upload_data_size;
            con_info->answerstring[con_info->data_size] = '\0';
            *upload_data_size = 0;
            return MHD_YES;
        } else {
            // TODO
            // THIS IS WHERE YOU HANDLE POST DATA
            char *result = route(router, method, url, con_info->answerstring);
            if (!result) {
                return send_page(connection, strdup("404 Not Found"));
            }
            if (con_info->answerstring) {
                free(con_info->answerstring);
                con_info->answerstring = NULL;
                con_info->data_size = 0;
            }
            return send_page(connection, result);
        }
    }

    return send_page(connection, strdup("405 Method Not Allowed"));
}

int main() {
    Router *router = router_init();
    if (!router) {
        fprintf(stderr, "Failed to initialize router\n");
        return 1;
    }

    register_route(router, GET, "/test", &another_route);
    register_route(router, GET, "/hello", &hello_from_router);
    register_route(router, POST, "/sync", &parse_sync);

    struct MHD_Daemon *daemon = MHD_start_daemon(
                                    MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD,
                                    PORT, NULL, NULL,
                                    &answer_to_connection, router,
                                    MHD_OPTION_NOTIFY_COMPLETED, request_completed,
                                    NULL, MHD_OPTION_END);
    if (!daemon) {
        fprintf(stderr, "Failed to start daemon\n");
        router_free(router);
        return 1;
    }

    printf("Server running on port %d. Press Enter to stop...\n", PORT);
    (void)getchar();

    MHD_stop_daemon(daemon);
    router_free(router);
    return 0;
}
