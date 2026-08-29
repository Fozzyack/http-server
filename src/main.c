#include "http/parser.h"
#include "log/log.h"
#include "server/server.h"
#include <stdlib.h>
#include <unistd.h>

void handle_client(int client_fd) { http_request req = {0}; 
    if(parse_http_request(&req, client_fd) != PARSE_OK) {
        log_message(LOG_ERROR, "huh");
    }
    
    log_message(LOG_INFO, "%s", req.method); 
    log_message(LOG_INFO, "%s", req.path); 
    log_message(LOG_INFO, "%s", req.protocol); 
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
