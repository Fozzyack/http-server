#include "routes/router.h"
#include "server/server.h"
#include "threadpool/threadpool.h"
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

    tcp_server_info server_info = {0};
    router router_info = {0};
    threadpool pool = {0};
    init_server(&server_info, 8080);
    tcp_server_status server_status = bind_tcp_server(&server_info);
    if (server_status != SERVER_OK) {
        return EXIT_FAILURE;
    }
    setup_router(&router_info);
    threadpool_start(&pool);
    server_status = listen_and_accept(server_info.socket_fd, &router_info, &pool);
    if (server_status != SERVER_OK) {
        return EXIT_FAILURE;
    }

    threadpool_stop(&pool);
    close(server_info.socket_fd);
    return EXIT_SUCCESS;
}
