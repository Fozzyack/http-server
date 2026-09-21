#include "http/http.h"
#include "log/log.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <errno.h>
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
    if (response == NULL) {
        return;
    }
    free(response->headers);
    free(response->body);
    response->headers = NULL;
    response->header_count = 0;
    response->body = NULL;
    response->body_size = 0;
}

http_response_status response_set_header(const char *key, const char *value, http_response *response) {

    if (!key || !value || !response) {
        log_message(LOG_ERROR, "response_set_header: key value or response not present");
        return RESPONSE_ERROR;
    }
    if (strlen(key) >= HEADER_NAME_LENGTH || strlen(value) >= HEADER_VALUE_LENGTH) {
        log_message(LOG_ERROR, "response_set_header: header field exceeds maximum length");
        return RESPONSE_ERROR;
    }

    // check if header already exists, if so update the header
    for (size_t i = 0; i < response->header_count; i++) {
        if (!strcmp(response->headers[i].name, key)) {
            memcpy(response->headers[i].value, value, strlen(value) + 1);
            return RESPONSE_OK;
        }
    }
    http_header *headers = realloc(response->headers, sizeof(http_header) * (response->header_count + 1));
    if (headers == NULL) {
        log_errno(LOG_ERROR, "realloc; response_set_header");
        return RESPONSE_ERROR;
    }
    response->headers = headers;
    memcpy(response->headers[response->header_count].name, key, strlen(key) + 1);
    memcpy(response->headers[response->header_count].value, value, strlen(value) + 1);
    response->header_count++;

    return RESPONSE_OK;
}

http_response_status response_set_json(const char *json_string, http_response *response) {
    if (json_string == NULL || response == NULL) {
        log_message(LOG_ERROR, "response_set_json: json string or response not present");
        return RESPONSE_ERROR;
    }

    size_t length = strlen(json_string);
    char length_str[32];
    snprintf(length_str, sizeof(length_str), "%zu", length);

    char *body = malloc(length + 1);
    if (body == NULL) {
        log_errno(LOG_ERROR, "malloc");
        return RESPONSE_ERROR;
    }
    memcpy(body, json_string, length + 1);

    if (response_set_header("Content-Type", "application/json", response) != RESPONSE_OK ||
        response_set_header("Content-Length", length_str, response) != RESPONSE_OK) {
        free(body);
        return RESPONSE_ERROR;
    }

    free(response->body);
    response->body = body;
    response->body_size = length;

    return RESPONSE_OK;
}

char *construct_response(const http_response *response, size_t *response_length) {
    if (response == NULL || response_length == NULL) {
        return NULL;
    }
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
            char *new_buffer = realloc(buffer, buffer_size);
            if (new_buffer == NULL) {
                log_errno(LOG_ERROR, "Failed to malloc for buffer");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
        offset += snprintf(buffer + offset, buffer_size - offset, "%s: %s\r\n", response->headers[i].name,
                           response->headers[i].value);
    }
    offset += snprintf(buffer + offset, buffer_size - offset, "\r\n");
    if (response->body) {
        while (offset + response->body_size > buffer_size) {
            buffer_size *= 2;
            char *new_buffer = realloc(buffer, buffer_size);
            if (new_buffer == NULL) {
                log_errno(LOG_ERROR, "Failed to malloc for buffer");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
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
    if (response_data == NULL) {
        return RESPONSE_ERROR;
    }

    size_t bytes_sent = 0;
    while (bytes_sent < buffer_size) {
        ssize_t b_sent = send(client_fd, response_data + bytes_sent, buffer_size - bytes_sent, 0);
        if (b_sent == -1) {
            if (errno == EINTR) {
                continue;
            }
            free(response_data);
            return RESPONSE_ERROR;
        }
        if (b_sent == 0) {
            free(response_data);
            return RESPONSE_ERROR;
        }

        bytes_sent += (size_t)b_sent;
    }
    free(response_data);
    return RESPONSE_OK;
}
