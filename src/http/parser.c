#include "http/parser.h"
#include "log/log.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

parse_status read_http_request(int socket_fd, http_request *request) {

    char *destinations[] = {
        request->method,
        request->path,
        request->protocol,
    };

    size_t capacities[] = {
        sizeof(request->method),
        sizeof(request->path),
        sizeof(request->protocol),
    };

    char buffer[BUFFER_SIZE];

    ssize_t bytes_read = read(socket_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        log_errno(LOG_ERROR, "read_http_request read");
        return PARSE_READ_ERROR;
    }

    buffer[bytes_read] = '\0';
    char *cursor = buffer;

    for (size_t i = 0; i < 3; i++) {
        char *start = cursor;

        while (*cursor != '\0' && *cursor != '\n' && *cursor != '\r' && *cursor != ' ') {
            cursor++;
        }

        size_t length = (size_t)(cursor - start);
        if (length == 0 || length >= capacities[i]) {
            log_message(LOG_ERROR, "length of request line too large");
            return PARSE_FIELD_TOO_BIG;
        }

        memcpy(destinations[i], start, length);
        destinations[i][length] = '\0';

        if (i < 2) {
            if (*cursor != ' ') {
                return PARSE_READ_ERROR;
            }
            cursor++;
        }
    }

    if (*cursor != '\0' && *cursor != '\r' && *cursor != '\n') {
        return PARSE_READ_ERROR;
    }

    return PARSE_READ_OK;
}
