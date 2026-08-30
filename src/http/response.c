#include "http/response.h"
#include <stdlib.h>
#include <string.h>

void init_response(http_response *response) {
    response->status = 0;
    memset(response->status_response, '\0', STATUS_RESPOSNE_LENGTH);
    response->headers = malloc(0);
    response->header_count = 0;
    response->body_buffer = malloc(0);
    response->body_buffer_size = 0;
}

void destroy_response(http_response *response) {
    free(response->headers);
    free(response->body_buffer);
}

http_response_status send_response(http_response *response, int client_fd) {
    init_response(response);

    destroy_response(response);
    return RESPONSE_OK;
}
