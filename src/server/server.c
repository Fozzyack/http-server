#include "server/server.h"
#include "log/log.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define LISTEN_BACKLOG 25

tcp_server_status init_server(tcp_server_info *server_info, int port) {

    if (!server_info) {
        log_message(LOG_ERROR, "server_info is NULL");
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
        log_message(LOG_ERROR, "socket error");
        return SERVER_SOCKET_INIT_ERROR;
    }
    server_info->socket_fd = fd;

    int reuse_addr = 1;
    int reuse_port = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) {
        perror("setsockopt");
        log_message(LOG_ERROR, "setsockopt error");
        return SERVER_SETSOCKOPT_ERROR;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port)) == -1) {
        perror("setsockopt");
        log_message(LOG_ERROR, "setsockopt error");
        return SERVER_SETSOCKOPT_ERROR;
    }

    if (bind(fd, (struct sockaddr *)server_info, sizeof(*server_info)) == -1) {
        perror("bind");
        log_message(LOG_ERROR, "bind error");
        return SERVER_BIND_ERROR;
    }

    if (listen(fd, LISTEN_BACKLOG) == -1) {
        perror("bind");
        log_message(LOG_ERROR, "listen error");
        return SERVER_BIND_ERROR;
    }

    return SERVER_OK;
}

tcp_server_status accept_client(int server_fd, int *client_fd) {
    struct sockaddr_in client_info = {0};
    socklen_t client_info_len = sizeof(client_info);

    int fd = accept(server_fd, (struct sockaddr *)&client_info, &client_info_len);
    if (fd == -1) {
        perror("accept");
        log_message(LOG_ERROR, "accept error");
        return SERVER_ACCEPT_ERROR;
    }

    *client_fd = fd;

    return SERVER_OK;
}
