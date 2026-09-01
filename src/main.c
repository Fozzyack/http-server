#include "http/parser.h"
#include "http/response.h"
#include "log/log.h"
#include "server/server.h"
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

void handle_client(int client_fd) {
    http_request req = {0};
    int status;
    if ((status = parse_http_request(&req, client_fd)) != PARSE_OK) {
        log_message(LOG_ERROR, "huh %d", status);
        return;
    }
}

void test_http_res(int client_fd) {
    http_response res = {0};
    init_response(&res);
    response_set_header("Content-Type", "application/json", &res);
    response_set_header("Content-Length", "32", &res);
    response_set_header("Connection", "keep-alive", &res);
    const char *json = "{\"message\":\"hello\"}";
    response_set_json(json, &res);
    send_response(client_fd, &res);
    destroy_response(&res);
}

int main(void) {

    tcp_server_info server_info = {0};
    tcp_server_status server_status = bind_tcp_server(&server_info, 8080);
    if (server_status != SERVER_OK) {
        return EXIT_FAILURE;
    }

    while (1) {
        int client_fd;
        server_status = accept_client(server_info.socket_fd, &client_fd);
        if (server_status == SERVER_ACCEPT_ERROR) {
            log_message(LOG_ERROR, "Failed to accept client");
            return EXIT_FAILURE;
        }
        test_http_res(client_fd);
        close(client_fd);
    }

    close(server_info.socket_fd);
    return EXIT_SUCCESS;
}
