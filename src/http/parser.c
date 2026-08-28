#include "http/parser.h"
#include "log/log.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

parse_status read_request(int socket_fd, char *buffer, size_t *end_of_buffer) {
    ssize_t bytes_read = read(socket_fd, buffer + *end_of_buffer, BUFFER_SIZE - 1 - *end_of_buffer);
    if (bytes_read <= 0) {
        log_errno(LOG_ERROR, "read http_request");
        return PARSE_READ_ERROR;
    }
    *end_of_buffer += bytes_read;
    buffer[*end_of_buffer] = '\0';
    return PARSE_OK;
}

parse_status align_buffer(size_t *start_of_buffer, size_t *end_of_buffer, char *buffer) {

    *end_of_buffer = (*end_of_buffer - *start_of_buffer);
    memmove(buffer, buffer + *start_of_buffer, *end_of_buffer);
    buffer[*end_of_buffer] = '\0';
    *start_of_buffer = 0;
    return PARSE_OK;
}

parse_status parse_request_line(http_request *request, size_t *request_line_property, char *buffer,
                                size_t *start_of_buffer, size_t *end_of_buffer, int *parsed_headers) {

    // Thoughts:
    // we need to keep track of what property we are up to
    // if we do not get to the end of the line we need to re-read

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

    char *cursor = buffer + *start_of_buffer;
    for (; *request_line_property < 3; (*request_line_property)++) {
        char *start = cursor;

        while (*cursor != '\n' && *cursor != '\r' && *cursor != ' ') {
            cursor++;
            if (*cursor == *(buffer + *end_of_buffer)) {
                // we are at the end of the buffer and need to read more
                break;
            }
        }

        size_t length = (size_t)(cursor - start);
        if (capacities[*request_line_property] <= length) {
            log_message(LOG_ERROR, "length greater than capacity");
            return PARSE_ERROR;
        }

        *start_of_buffer += length;
        memcpy(destinations[*request_line_property], start, length);
        destinations[*request_line_property][length] = '\0';

        if (*request_line_property == 2) {
            *parsed_headers = 1;
        }
    }

    align_buffer(start_of_buffer, end_of_buffer, buffer);
    if (*cursor != '\0' && *cursor != '\r' && *cursor != '\n') {
        return PARSE_INCOMPLETE;
    }

    return PARSE_OK;
}

parse_status read_http_request(int socket_fd, http_request *request) {

    char buffer[BUFFER_SIZE] = {0};
    int parsed_request_line = 0;
    size_t request_line_property = 0;
    int parsed_headers = 0;
    size_t buffer_start = 0;
    size_t buffer_end = 0;

    parse_status status;

    while (!parsed_request_line) {
        if (read_request(socket_fd, buffer, &buffer_end) != PARSE_OK) {
            return PARSE_READ_ERROR;
        }

        if (!parsed_request_line) {
            status = parse_request_line(request, &request_line_property, buffer, &buffer_start, &buffer_end,
                                        &parsed_headers);
            if (status == PARSE_INCOMPLETE || status == PARSE_OK) {
                continue;
            } else {
                return PARSE_ERROR;
            }
        }
        if (!parsed_headers) {
            break; // not yet implemented
        }
    }
    return PARSE_OK;
}
