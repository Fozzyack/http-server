#include "http/response.h"
#include "http/parser.h"
#include "log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_response(http_response *response) {
    printf("init res structure\n");
    response->status = 0;
    memset(response->status_response, '\0', STATUS_RESPOSNE_LENGTH);
    response->headers = calloc(1, sizeof(http_header));
    if (!response->headers) {
        log_errno(LOG_ERROR, "calloc");
    }
    response->header_count = 0;
    response->body = malloc(sizeof(char) * 64);
    if (!response->body) {
        log_errno(LOG_ERROR, "malloc");
    }
    response->body_size = 0;
}

void destroy_response(http_response *response) {
    printf("destroying  response\n");
    free(response->headers);
    free(response->body_buffer);
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
