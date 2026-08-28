#ifndef PARSER_H
#define PARSER_H

#define METHOD_LENGTH 8
#define REQUEST_TARGET_LENGTH 2048
#define PROTOCOL_LENGTH 16

typedef enum {
    PARSE_READ_OK,
    PARSE_READ_ERROR,
    PARSE_SCAN_ERROR,
    PARSE_FIELD_TOO_BIG,
} parse_status;

typedef struct {
    char method[METHOD_LENGTH];
    char path[REQUEST_TARGET_LENGTH];
    char protocol[PROTOCOL_LENGTH];
} http_request;

parse_status read_http_request(int socket_fd, http_request *request);

#endif // PARSER_H
