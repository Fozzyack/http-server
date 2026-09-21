#include "http/http.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void set_buffer(http_request_buffer *buffer, const char *text) {
    size_t length = strlen(text);

    assert(length < BUFFER_SIZE);
    memcpy(buffer->buffer, text, length + 1);
    buffer->start = 0;
    buffer->end = length;
}

int main(void) {
    http_request request;
    http_request_buffer buffer;
    size_t eol = 0;

    init_request_info(&request, &buffer);
    assert(request.header_count == 0);
    assert(buffer.start == 0);
    assert(buffer.end == 0);

    set_buffer(&buffer, "GET /healthcheck HTTP/1.1\r\nHost: example.com\r\n");
    assert(find_line(&buffer, &eol) == PARSE_OK);
    assert(parse_request_line(&request, &buffer, &eol) == PARSE_OK);
    assert(strcmp(request.method, "GET") == 0);
    assert(strcmp(request.path, "/healthcheck") == 0);
    assert(strcmp(request.protocol, "HTTP/1.1") == 0);

    assert(find_line(&buffer, &eol) == PARSE_OK);
    assert(parse_header(&request, &buffer, &eol) == PARSE_OK);
    assert(request.header_count == 1);
    assert(strcmp(request.headers[0].name, "Host") == 0);
    assert(strcmp(request.headers[0].value, "example.com") == 0);

    set_buffer(&buffer, "GET /healthcheck HTTP/1.1");
    assert(find_line(&buffer, &eol) == PARSE_LINE_NOT_FOUND);

    set_buffer(&buffer, "GET /healthcheck\r\n");
    assert(find_line(&buffer, &eol) == PARSE_OK);
    assert(parse_request_line(&request, &buffer, &eol) == PARSE_REQUEST_FIELD_ERROR);

    set_buffer(&buffer, "Host example.com\r\n");
    assert(find_line(&buffer, &eol) == PARSE_OK);
    assert(parse_header(&request, &buffer, &eol) == PARSE_HEADER_ERROR);

    return EXIT_SUCCESS;
}
