#ifndef PARSER_H
#define PARSER_H

#define METHOD_LENGTH 8
#define REQUEST_TARGET_LENGTH 2048
#define PROTOCOL_LENGTH 16

typedef struct {
    char method[METHOD_LENGTH];
    char path[REQUEST_TARGET_LENGTH];
    char protocol[PROTOCOL_LENGTH];
} http_request;

#endif // PARSER_H
