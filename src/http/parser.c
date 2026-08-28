#include "http/parser.h"
#include "log/log.h"
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

parse_status read_http_request(int socket_fd, http_request *request) {
    char buffer[BUFFER_SIZE];

    ssize_t bytes_read = read(socket_fd, &buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        log_errno(LOG_WARN, "read - reading failed or connection closed");
        return PARSE_READ_ERROR;
    }

    buffer[bytes_read] = '\0';

    if (sscanf(buffer, "%7s, %2047s %15s", request->method, request->path, request->protocol) != 3) {
        log_message(LOG_ERROR, "sscanf failed to parse request line");
        return PARSE_SCAN_ERROR;
    }

    return PARSE_READ_OK;
}
