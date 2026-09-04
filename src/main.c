#include "server/server.h"
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

    tcp_server_info server_info = {0};
    tcp_server_status server_status = bind_tcp_server(&server_info, 8080);
    if (server_status != SERVER_OK) {
        return EXIT_FAILURE;
    }
    server_status = listen_and_accept(server_info.socket_fd);
    if (server_status != SERVER_OK) {
        return EXIT_FAILURE;
    }

    close(server_info.socket_fd);
    return EXIT_SUCCESS;
}
