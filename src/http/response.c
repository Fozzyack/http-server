#include "http/response.h"
#include "http/parser.h"
#include "log/log.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_response(http_response *response) {
    printf("init res structure\n");
    response->status = 200;
    memset(response->status_response, '\0', STATUS_RESPOSNE_LENGTH);
    response->headers = NULL;
    response->header_count = 0;
    response->body = NULL;
    response->body_size = 0;
}

void destroy_response(http_response *response) {
    printf("destroying  response\n");
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
    response->body = calloc(length, sizeof(char));
    if (!response->body) {
        log_errno(LOG_ERROR, "malloc");
    }
    response_set_header("Content-Type", "application/json", response);
    response_set_header("Content-Length", length_str, response);
    memcpy(response->body, json_string, length);
    response->body[length + 1] = '\0';

    return RESPONSE_OK;
}
