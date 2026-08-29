#include "http/parser.h"
#include "log/log.h"
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

parse_status read_from_socket(int socket_fd, char *buffer, size_t *buffer_end) {
    size_t emtpy_space = BUFFER_SIZE - 1 - *buffer_end;
    ssize_t bytes_read = read(socket_fd, buffer + *buffer_end, emtpy_space);
    if (bytes_read < 0) {
        log_errno(LOG_ERROR, "read");
        return PARSE_READ_ERROR;
    } else if (bytes_read == 0) {
        log_errno(LOG_ERROR, "read");
        return PARSE_READ_SOCKET_DISCONNECTED;
    }
    *buffer_end += bytes_read;
    buffer[*buffer_end] = '\0';
    return PARSE_OK;
}

parse_status align_buffer(char *buffer, size_t *buffer_start, size_t *buffer_end) {
    size_t unread = *buffer_end - *buffer_start;

    memmove(buffer, buffer + *buffer_start, unread);
    *buffer_start = 0;
    *buffer_end = unread;
    buffer[*buffer_end] = '\0';

    return PARSE_OK;
}

parse_status find_line(char *buffer, size_t *buffer_end, size_t *eol) {

    // Buffer must have at least 2 characters \r\n
    // otherwise read again
    // do not return line exists without \r\n present in buffer

    if (*buffer_end < 2) {
        return PARSE_LINE_NOT_FOUND;
    }
    for (size_t i = 1; i < *buffer_end; i++) {
        if (buffer[i - 1] == '\r' && buffer[i] == '\n') {
            // we have detected a line
            *eol = i + 1;
            return PARSE_OK;
        }
    }

    return PARSE_LINE_NOT_FOUND;
}

// When running find_line would have made sure we have a line in buffer to process
parse_status parse_request_line(http_request *request, char *buffer, size_t *buffer_start, size_t *buffer_end) {
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

    char *cursor = buffer;
    for (size_t i = 0; i < 3; i++) {

        char *start = cursor;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\r' && *cursor != '\n') {
            cursor++;
        }

        size_t length = (size_t)(cursor - start);
        *buffer_start += length;
        if (length >= capacities[i]) {
            log_message(LOG_ERROR, "request line header field exceeds max width");
            return PARSE_LINE_FIELD_EXCEEDS_MAX_WIDTH;
        }
        memcpy(destinations[i], start, length);
        destinations[i][length] = '\0';

        if (i < 2) {
            if (*cursor != ' ') {
                return PARSE_LINE_ERROR;
            }
            cursor++;
        }
    }
    if (*cursor != '\r') {
        log_message(LOG_ERROR, "request line header field ends incorrectly");
        return PARSE_LINE_ERROR;
    }
    align_buffer(buffer, buffer_start, buffer_end);

    return PARSE_OK;
}

parse_status parse_http_request(http_request *request, int client_fd) {

    char buffer[BUFFER_SIZE];
    size_t buffer_start = 0;
    size_t buffer_end = 0;

    int has_req_line = 0;
    int has_headers = 1; // set as 1 for now as it is not yet implemented

    size_t eol = 0; // holds where the end of the line is
    while (!has_req_line || !has_headers) {

        parse_status status = find_line(buffer, &buffer_end, &eol);
        if (status != PARSE_LINE_NOT_FOUND) {
            // logic here
            if (!has_req_line) {
                if (parse_request_line(request, buffer, &buffer_start, &buffer_end) == PARSE_OK) {
                    has_req_line = 1;
                } else {
                    return PARSE_ERROR;
                }
            } else if (!has_headers) {
                break;
            }
        } else {
            status = read_from_socket(client_fd, buffer, &buffer_end);
            if (status == PARSE_READ_SOCKET_DISCONNECTED) {
                return PARSE_READ_SOCKET_DISCONNECTED;
            }
        }
    }

    return PARSE_OK;
}
