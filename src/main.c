#include "http/parser.h"
#include "http/response.h"
#include "log/log.h"
#include "server/server.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handle_client(int client_fd) {
    http_request req = {0};
    int status;
    if ((status = parse_http_request(&req, client_fd)) != PARSE_OK) {
        log_message(LOG_ERROR, "huh %d", status);
        return;
    }

    // log_message(LOG_INFO, "%s %s %s", req.method, req.path, req.protocol);
    // for (size_t i = 0; i < req.header_count; i++) {
    //     log_message(LOG_INFO, "%s: %s", req.headers[i].name, req.headers[i].value);
    // }
}

void test_http_res(int client_fd) {
    http_response res = {0};
    init_response(&res);
    response_set_header("Content-Type", "application/json", &res);
    response_set_header("Content-Length", "32", &res);
    response_set_header("Connection", "keep-alive", &res);
    const char *json = "{\"message\":\"hello\"}";
    response_set_json(json, &res);
    // for (size_t i = 0; i < res.header_count; i++) {
    //     log_message(LOG_INFO, "%s: %s", res.headers[i].name, res.headers[i].value);
    // }
    // for (size_t i = 0; i < res.body_size; i++) {
    //     putc(res.body[i], stderr);
    // }
    // putc('\n', stderr);
    send_response(client_fd, &res);
    destroy_response(&res);
}

int main(void) {

    tcp_server_info server_info = {0};
    tcp_server_status server_status = bind_tcp_server(&server_info, 8080);
    if (server_status != SERVER_OK) {
        return EXIT_FAILURE;
    }

    log_message(LOG_INFO, "Server started on port %d", 8080);

    int client_fd;
    server_status = accept_client(server_info.socket_fd, &client_fd);
    if (server_status != SERVER_OK) {
        return EXIT_FAILURE;
    }

    handle_client(client_fd);
    test_http_res(client_fd);
    close(client_fd);
    close(server_info.socket_fd);

    return EXIT_SUCCESS;
}
