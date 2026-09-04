#include "http/http.h"
#include "log/log.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void init_response(http_response *response) {
    response->status = 200;
    memset(response->status_response, 0, STATUS_RESPOSNE_LENGTH);
    response->status_response[0] = 'O';
    response->status_response[1] = 'K';
    response->headers = NULL;
    response->header_count = 0;
    response->body = NULL;
    response->body_size = 0;
}

void destroy_response(http_response *response) {
    free(response->headers);
    free(response->body);
}

http_response_status response_set_header(const char *key, const char *value, http_response *response) {

    if (!key || !value || !response) {
        log_message(LOG_ERROR, "response_set_header: key value or response not present");
        return RESPONSE_ERROR;
    }

    // check if header already exists, if so update the header
    for (size_t i = 0; i < response->header_count; i++) {
        if (!strcmp(response->headers[i].name, key)) {
            strcpy(response->headers[i].value, value);
            return RESPONSE_OK;
        }
    }
    // else make new header
    response->headers = realloc(response->headers, sizeof(http_header) * (response->header_count + 1));
    strcpy(response->headers[response->header_count].name, key);
    strcpy(response->headers[response->header_count].value, value);
    response->header_count++;

    return RESPONSE_OK;
}

http_response_status response_set_json(const char *json_string, http_response *response) {

    char *cursor = (char *)json_string;
    char *start = cursor;
    while (*cursor != '\0') {
        cursor++;
    }

    size_t length = (size_t)(cursor - start);
    char length_str[4096];
    sprintf(length_str, "%zu", length);

    response->body_size = length;
    response->body = malloc(length + 1);
    if (!response->body) {
        log_errno(LOG_ERROR, "malloc");
        return RESPONSE_ERROR;
    }
    response_set_header("Content-Type", "application/json", response);
    response_set_header("Content-Length", length_str, response);
    memcpy(response->body, json_string, length + 1);
    response->body[length] = '\0';

    return RESPONSE_OK;
}

char *construct_response(const http_response *response, size_t *response_length) {
    size_t buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        log_errno(LOG_ERROR, "Failed to malloc for buffer");
        return NULL;
    }

    size_t offset = snprintf(buffer, buffer_size, "HTTP/1.1 %d %s\r\n", response->status, response->status_response);
    for (size_t i = 0; i < response->header_count; i++) {
        size_t header_length =
            (size_t)snprintf(NULL, 0, "%s: %s\r\n", response->headers[i].name, response->headers[i].value);
        while (offset + header_length + 1 > buffer_size) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                log_errno(LOG_ERROR, "Failed to malloc for buffer");
                return NULL;
            }
        }
        offset += snprintf(buffer + offset, buffer_size - offset, "%s: %s\r\n", response->headers[i].name,
                           response->headers[i].value);
    }
    offset += snprintf(buffer + offset, buffer_size - offset, "\r\n");
    if (response->body) {
        while (offset + response->body_size > buffer_size) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                log_errno(LOG_ERROR, "Failed to malloc for buffer");
                return NULL;
            }
        }
        memcpy(buffer + offset, response->body, response->body_size);
        offset += response->body_size;
    }
    *response_length = offset;
    return buffer;
}

http_response_status send_response(int client_fd, http_response *response) {
    size_t buffer_size = 0;
    char *response_data = construct_response(response, &buffer_size);

    size_t bytes_sent = 0;
    while (bytes_sent < buffer_size) {
        ssize_t b_sent = send(client_fd, response_data + bytes_sent, buffer_size - bytes_sent, 0);
        if (b_sent <= 0) {
            log_errno(LOG_ERROR, "Failed to send Response");
            break;
        }

        bytes_sent += (size_t)b_sent;
    }
    free(response_data);
    return RESPONSE_OK;
}
