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


#if defined(_MSC_VER) && _MSC_VER + 0 <= 1800
/* Substitution is OK while return value is not used */
#define snprintf _snprintf
#endif

#define PORT            8888
#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     20
#define MAXANSWERSIZE   512

#define GET             0
#define POST            1

struct connection_info_struct
{
    int connectiontype;
    char *answerstring;
    struct MHD_PostProcessor *postprocessor;
};

const char *askpage =
    "<html><body>\
                       What's your name, Sir?<br>\
                       <form action=\"/namepost\" method=\"post\">\
                       <input name=\"name\" type=\"text\">\
                       <input type=\"submit\" value=\" Send \"></form>\
                       </body></html>";

const char *greetingpage =
    "<html><body><h1>Welcome, %s!</center></h1></body></html>";

const char *errorpage =
    "<html><body>This doesn't seem to be right.</body></html>";


static enum MHD_Result
send_page (struct MHD_Connection *connection, char *page)
{
    enum MHD_Result ret;
    struct MHD_Response *response;


    response =
    MHD_create_response_from_buffer (strlen (page), (void *) page,
                                     MHD_RESPMEM_MUST_FREE);
    if (! response)
        return MHD_NO;

    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);


    return ret;
}


static enum MHD_Result
iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
              const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off,
              size_t size)
{
    struct connection_info_struct *con_info = coninfo_cls;
    (void) kind;               /* Unused. Silent compiler warning. */
    (void) filename;           /* Unused. Silent compiler warning. */
    (void) content_type;       /* Unused. Silent compiler warning. */
    (void) transfer_encoding;  /* Unused. Silent compiler warning. */
    (void) off;                /* Unused. Silent compiler warning. */

    if (0 == strcmp (key, "name"))
    {
        if ((size > 0) && (size <= MAXNAMESIZE))
        {
            char *answerstring;
            answerstring = malloc (MAXANSWERSIZE);
            if (! answerstring)
                return MHD_NO;

            snprintf (answerstring, MAXANSWERSIZE, greetingpage, data);
            con_info->answerstring = answerstring;
            printf("Iterating Post found: %s\n", con_info->answerstring);
        }
        else
            con_info->answerstring = NULL;

        return MHD_NO;
    }

    return MHD_YES;
}


static void
request_completed (void *cls, struct MHD_Connection *connection,
                   void **con_cls, enum MHD_RequestTerminationCode toe)
{
    struct connection_info_struct *con_info = *con_cls;
    (void) cls;         /* Unused. Silent compiler warning. */
    (void) connection;  /* Unused. Silent compiler warning. */
    (void) toe;         /* Unused. Silent compiler warning. */

    if (NULL == con_info)
        return;

    if (con_info->connectiontype == POST)
    {
        MHD_destroy_post_processor (con_info->postprocessor);
        if (con_info->answerstring)
            free (con_info->answerstring);
    }

    free (con_info);
    *con_cls = NULL;
}


static enum MHD_Result
answer_to_connection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
    (void) cls;               /* Unused. Silent compiler warning. */
    // (void) url;               /* Unused. Silent compiler warning. */
    (void) version;           /* Unused. Silent compiler warning. */

    char *result = route(method, url);

    // // Send Result Dynamically
    return send_page (connection, result);
}

int
main ()
{
    router_init();
    // Play around with routes
    register_route(&another_route, "/test");
    register_route(&hello_from_router, "/hello");

    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon (MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD,
                               PORT, NULL, NULL,
                               &answer_to_connection, NULL,
                               MHD_OPTION_NOTIFY_COMPLETED, request_completed,
                               NULL, MHD_OPTION_END);
    if (NULL == daemon)
        return 1;

    (void) getchar ();

    MHD_stop_daemon (daemon);

    return 0;
}
