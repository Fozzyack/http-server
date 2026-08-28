#include "http/parser.h"
#include "log/log.h"
#include "server/server.h"
#include <stdlib.h>
#include <unistd.h>

void handle_client(int client_fd) {
    http_request req = {0};

    if (read_http_request(client_fd, &req) != PARSE_READ_OK) {
        log_message(LOG_ERROR, "Failed to parse HTTP request");
        close(client_fd);
        return;
    }
    log_message(LOG_INFO, "HTTP request parsed");
    log_message(LOG_INFO, "method: %s", req.method);
    log_message(LOG_INFO, "path: %s", req.path);
    log_message(LOG_INFO, "protocol: %s", req.protocol);
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
    close(client_fd);

    close(server_info.socket_fd);

    return EXIT_SUCCESS;
}
