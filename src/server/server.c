#include "server/server.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

#define LISTEN_BACKLOG 25

tcp_server_status init_server(tcp_server_info *server_info, int port) {

    if (!server_info) {
        fprintf(stderr, "server_info struct does not exist\n");
        return SERVER_INFO_DOES_NOT_EXIST;
    }

    server_info->address.sin_family = AF_INET;
    server_info->address.sin_port = htons(port);
    server_info->address.sin_addr.s_addr = INADDR_ANY;

    return SERVER_OK;
}

tcp_server_status bind_tcp_server(tcp_server_info *server_info, int port) {

    tcp_server_status status;
    if ((status = init_server(server_info, port)) != SERVER_OK) {
        return status;
    }

    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        perror("socket");
        return SERVER_SOCKET_INIT_ERROR;
    }
    server_info->socket_fd = fd;

    int reuse_addr = 1;
    int reuse_port = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) {
        perror("setsockopt");
        return SERVER_SETSOCKOPT_ERROR;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port)) == -1) {
        perror("setsockopt");
        return SERVER_SETSOCKOPT_ERROR;
    }

    if (bind(fd, (struct sockaddr *)server_info, sizeof(*server_info)) == -1) {
        perror("bind");
        return SERVER_BIND_ERROR;
    }

    if (listen(fd, LISTEN_BACKLOG) == -1) {
        perror("bind");
        return SERVER_BIND_ERROR;
    }

    return SERVER_OK;
}
