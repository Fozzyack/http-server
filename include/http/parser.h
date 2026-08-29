#ifndef PARSER_H
#define PARSER_H

#define METHOD_LENGTH 8
#define REQUEST_TARGET_LENGTH 2048
#define PROTOCOL_LENGTH 16

typedef enum {
    PARSE_OK,
    PARSE_ERROR,
    PARSE_READ_ERROR,
    PARSE_LINE_NOT_FOUND,
    PARSE_LINE_FIELD_EXCEEDS_MAX_WIDTH,
    PARSE_LINE_ERROR,
    PARSE_READ_SOCKET_DISCONNECTED,
    PARSE_FIELD_TOO_BIG,
} parse_status;

typedef struct {
    char method[METHOD_LENGTH];
    char path[REQUEST_TARGET_LENGTH];
    char protocol[PROTOCOL_LENGTH];
} http_request;

parse_status parse_http_request(http_request *request, int client_fd);

#endif // PARSER_H
