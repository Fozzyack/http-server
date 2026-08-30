#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define METHOD_LENGTH 8
#define REQUEST_TARGET_LENGTH 2048
#define PROTOCOL_LENGTH 16

#define MAX_HEADERS 100
#define HEADER_NAME_LENGTH 64
#define HEADER_VALUE_LENGTH 1024

#define BUFFER_SIZE 8192

typedef enum parse_status {
    PARSE_OK,
    PARSE_ERROR,
    PARSE_READ_ERROR,
    PARSE_READ_BUFFER_FULL,
    PARSE_LINE_NOT_FOUND,
    PARSE_REQUEST_FIELD_EXCEEDS_MAX_LENGTH,
    PARSE_REQUEST_FIELD_ERROR,
    PARSE_HEADER_FIELD_EXCEEDS_MAX_LENGTH,
    PARSE_HEADER_EXCEEDS_MAX_HEADERS,
    PARSE_HEADER_ERROR,
    PARSE_READ_SOCKET_DISCONNECTED,
    PARSE_FIELD_TOO_BIG,
} parse_status;

typedef struct http_request_buffer {
    char buffer[BUFFER_SIZE];
    size_t start;
    size_t end;
} http_request_buffer;

typedef struct http_request_header {
    char name[HEADER_NAME_LENGTH];
    char value[HEADER_VALUE_LENGTH];
} http_request_header;

typedef struct http_request {
    char method[METHOD_LENGTH];
    char path[REQUEST_TARGET_LENGTH];
    char protocol[PROTOCOL_LENGTH];

    http_request_header headers[MAX_HEADERS];
    size_t header_count;
} http_request;

parse_status parse_http_request(http_request *request, int client_fd);

#endif // PARSER_H
