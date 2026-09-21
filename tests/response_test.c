#include "http/http.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    http_response response;
    char long_name[HEADER_NAME_LENGTH + 1];
    int sockets[2];
    char received[128] = {0};
    const char expected[] = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 2\r\n\r\n{}";

    init_response(&response);
    assert(response.status == 200);
    assert(strcmp(response.status_response, "OK") == 0);
    assert(response.header_count == 0);
    assert(response.body == NULL);

    assert(response_set_header("X-Test", "one", &response) == RESPONSE_OK);
    assert(response_set_header("X-Test", "two", &response) == RESPONSE_OK);
    assert(response.header_count == 1);
    assert(strcmp(response.headers[0].value, "two") == 0);

    memset(long_name, 'a', sizeof(long_name) - 1);
    long_name[sizeof(long_name) - 1] = '\0';
    assert(response_set_header(long_name, "value", &response) == RESPONSE_ERROR);
    assert(response_set_json(NULL, &response) == RESPONSE_ERROR);
    destroy_response(&response);

    init_response(&response);
    assert(response_set_json("{}", &response) == RESPONSE_OK);
    assert(response.body_size == 2);
    assert(strcmp(response.body, "{}") == 0);
    assert(response.header_count == 2);

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    assert(send_response(sockets[0], &response) == RESPONSE_OK);
    assert(read(sockets[1], received, sizeof(received)) == (ssize_t)(sizeof(expected) - 1));
    assert(memcmp(received, expected, sizeof(expected) - 1) == 0);
    assert(send_response(-1, &response) == RESPONSE_ERROR);

    close(sockets[0]);
    close(sockets[1]);
    destroy_response(&response);
    return EXIT_SUCCESS;
}
