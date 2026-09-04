#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#define METHOD_LENGTH 8
#define REQUEST_TARGET_LENGTH 2048
#define PROTOCOL_LENGTH 16

#define MAX_HEADERS 100
#define HEADER_NAME_LENGTH 64
#define HEADER_VALUE_LENGTH 1024

#define BUFFER_SIZE 8192

#define STATUS_RESPOSNE_LENGTH 64

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

typedef enum http_response_status {
    RESPONSE_OK,
    RESPONSE_ERROR,
} http_response_status;

typedef struct http_request_buffer {
    char buffer[BUFFER_SIZE];
    size_t start;
    size_t end;
} http_request_buffer;

typedef struct http_request_header {
    char name[HEADER_NAME_LENGTH];
    char value[HEADER_VALUE_LENGTH];
} http_header;

typedef struct http_request {
    char method[METHOD_LENGTH];
    char path[REQUEST_TARGET_LENGTH];
    char protocol[PROTOCOL_LENGTH];

    http_header headers[MAX_HEADERS];
    size_t header_count;
} http_request;

typedef struct http_response {

    int status;
    char status_response[STATUS_RESPOSNE_LENGTH];

    http_header *headers;
    size_t header_count;

    char *body;
    size_t body_size;

} http_response;

parse_status parse_http_request(http_request *request, int client_fd);
http_response_status response_set_header(const char *key, const char *value, http_response *response);
http_response_status response_set_json(const char *json_string, http_response *response);
http_response_status send_response(int client_fd, http_response *response);
void destroy_response(http_response *response);
void init_response(http_response *response);

#endif // HTTP_H
