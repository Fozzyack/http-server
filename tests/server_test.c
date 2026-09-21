#include "server/server.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    tcp_server_info server_info = {0};
    struct sockaddr_in address = {0};
    socklen_t address_size = sizeof(address);
    int is_listening = 0;
    socklen_t option_size = sizeof(is_listening);

    assert(init_server(NULL, 8080) == SERVER_INFO_DOES_NOT_EXIST);
    assert(init_server(&server_info, 0) == SERVER_OK);
    assert(server_info.socket_fd == -1);
    assert(server_info.address.sin_family == AF_INET);
    assert(server_info.address.sin_port == htons(0));
    assert(server_info.address.sin_addr.s_addr == INADDR_ANY);

    assert(bind_tcp_server(NULL) == SERVER_INFO_DOES_NOT_EXIST);
    assert(bind_tcp_server(&server_info) == SERVER_OK);
    assert(getsockname(server_info.socket_fd, (struct sockaddr *)&address, &address_size) == 0);
    assert(ntohs(address.sin_port) != 0);
    assert(getsockopt(server_info.socket_fd, SOL_SOCKET, SO_ACCEPTCONN, &is_listening, &option_size) == 0);
    assert(is_listening == 1);

    close(server_info.socket_fd);
    return EXIT_SUCCESS;
}
