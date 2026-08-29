#include "http/parser.h"
#include "log/log.h"
#include <stddef.h>
#include <string.h>
#include <unistd.h>

parse_status read_from_socket(int socket_fd, http_request_buffer *req_buffer) {
    size_t emtpy_space = BUFFER_SIZE - 1 - req_buffer->end;
    ssize_t bytes_read = read(socket_fd, req_buffer->buffer + req_buffer->end, emtpy_space);
    if (bytes_read < 0) {
        log_errno(LOG_ERROR, "read");
        return PARSE_READ_ERROR;
    } else if (bytes_read == 0) {
        log_errno(LOG_ERROR, "read");
        return PARSE_READ_SOCKET_DISCONNECTED;
    }
    req_buffer->end += bytes_read;
    req_buffer->buffer[req_buffer->end] = '\0';
    return PARSE_OK;
}

parse_status align_buffer(http_request_buffer *req_buffer) {
    size_t unread = req_buffer->end - req_buffer->start;

    memmove(req_buffer->buffer, req_buffer->buffer + req_buffer->start, unread);
    req_buffer->start = 0;
    req_buffer->end = unread;
    req_buffer->buffer[req_buffer->end] = '\0';

    return PARSE_OK;
}

parse_status find_line(http_request_buffer *req_buffer, size_t *eol) {

    // Buffer must have at least 2 characters \r\n
    // otherwise read again
    // do not return line exists without \r\n present in buffer

    if (req_buffer->end < 2) {
        return PARSE_LINE_NOT_FOUND;
    }
    for (size_t i = 1; i < req_buffer->end; i++) {
        if (req_buffer->buffer[i - 1] == '\r' && req_buffer->buffer[i] == '\n') {
            // we have detected a line
            *eol = i + 1;
            return PARSE_OK;
        }
    }

    return PARSE_LINE_NOT_FOUND;
}

// When running find_line would have made sure we have a line in buffer to process
parse_status parse_request_line(http_request *request, http_request_buffer *req_buffer) {
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

    char *cursor = req_buffer->buffer;
    for (size_t i = 0; i < 3; i++) {

        char *start = cursor;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\r' && *cursor != '\n') {
            cursor++;
        }

        size_t length = (size_t)(cursor - start);
        req_buffer->start += length;
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
    align_buffer(req_buffer);

    return PARSE_OK;
}

parse_status parse_http_request(http_request *request, int client_fd) {

    struct http_request_buffer req_buffer = {
        .buffer = {0},
        .end = 0,
        .start = 0,
    };

    int has_req_line = 0;
    int has_headers = 1; // set as 1 for now as it is not yet implemented

    size_t eol = 0; // holds where the end of the line is
    while (!has_req_line || !has_headers) {

        parse_status status = find_line(&req_buffer, &eol);
        if (status != PARSE_LINE_NOT_FOUND) {
            // logic here
            if (!has_req_line) {
                if (parse_request_line(request, &req_buffer) == PARSE_OK) {
                    has_req_line = 1;
                } else {
                    return PARSE_ERROR;
                }
            } else if (!has_headers) {
                break;
            }
        } else {
            status = read_from_socket(client_fd, &req_buffer);
            if (status == PARSE_READ_SOCKET_DISCONNECTED) {
                return PARSE_READ_SOCKET_DISCONNECTED;
            }
        }
    }

    return PARSE_OK;
}
