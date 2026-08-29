#include "http/parser.h"
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

parse_status read_from_socket(int socket_fd, char *buffer, size_t *buffer_end) {
    size_t emtpy_space = BUFFER_SIZE - 1 - *buffer_end;
    ssize_t bytes_read = read(socket_fd, buffer + *buffer_end, emtpy_space);
    if (bytes_read < 0) {
        return PARSE_READ_ERROR;
    } else if (bytes_read == 0) {
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

parse_status parse_http_request(http_request *request, int client_fd) {

    char buffer[BUFFER_SIZE];
    size_t buffer_start = 0;
    size_t buffer_end = 0;

    int has_req_line = 0;
    int has_headers = 0;

    while (!has_req_line || !has_headers) {
        size_t eol = 0;
        parse_status status = find_line(buffer, &buffer_end, &eol);
        if (status != PARSE_LINE_NOT_FOUND) {
            // logic here
        } else {
            status = read_from_socket(client_fd, buffer, &buffer_end);
            if (status == PARSE_READ_SOCKET_DISCONNECTED) {
            }
            return PARSE_READ_SOCKET_DISCONNECTED;
        }
    }

    return PARSE_OK;
}
