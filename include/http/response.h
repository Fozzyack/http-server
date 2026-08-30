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
    int status_response[STATUS_RESPOSNE_LENGTH];

    http_header *headers;
    size_t header_count;

    char *body_buffer;
    size_t body_buffer_size;

} http_response;

#endif // !RESPONSE_H
