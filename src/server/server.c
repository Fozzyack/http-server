#include "server/server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

tcp_server_status init_server(tcp_server_info *server_info, int port) {

    if (!server_info) {
        fprintf(stderr, "server_info struct does not exist\n");
        return SERVER_ERROR;
    }

    server_info->address.sin_family = AF_INET;
    server_info->address.sin_port = htons(port);
    server_info->address.sin_addr.s_addr = INADDR_ANY;

    return SERVER_OK;
}
