#include "log/log.h"
#include "server/server.h"
#include <stdlib.h>
#include <unistd.h>

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

    close(client_fd);
    close(server_info.socket_fd);

    return EXIT_SUCCESS;
}
