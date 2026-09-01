#ifndef RESPONSE_H
#define RESPONSE_H

#include "http/parser.h"
#include <stddef.h>

#define STATUS_RESPOSNE_LENGTH 64

typedef enum http_response_status {
    RESPONSE_OK,
    RESPONSE_ERROR,
} http_response_status;

typedef struct http_response {

    int status;
    char status_response[STATUS_RESPOSNE_LENGTH];

    http_header *headers;
    size_t header_count;

    char *body;
    size_t body_size;

} http_response;

http_response_status response_set_header(const char *key, const char *value, http_response *response);
http_response_status response_set_json(const char *json_string, http_response *response);
http_response_status send_response(int client_fd, http_response *response);
void destroy_response(http_response *response);
void init_response(http_response *response);

#endif // !RESPONSE_H
