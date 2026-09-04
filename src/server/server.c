#include "server/server.h"
#include "log/log.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define LISTEN_BACKLOG 25
#define MAX_EVENTS 100

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
        log_errno(LOG_ERROR, "socket");
        return SERVER_SOCKET_INIT_ERROR;
    }
    server_info->socket_fd = fd;

    int reuse_addr = 1;
    int reuse_port = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) {
        perror("setsockopt");
        log_errno(LOG_ERROR, "setsockopt");
        return SERVER_SETSOCKOPT_ERROR;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port)) == -1) {
        perror("setsockopt");
        log_errno(LOG_ERROR, "setsockopt");
        return SERVER_SETSOCKOPT_ERROR;
    }

    if (bind(fd, (struct sockaddr *)&(server_info->address), sizeof(server_info->address)) == -1) {
        perror("bind");
        log_errno(LOG_ERROR, "bind");
        return SERVER_BIND_ERROR;
    }

    if (listen(fd, LISTEN_BACKLOG) == -1) {
        perror("bind");
        log_errno(LOG_ERROR, "listen");
        return SERVER_BIND_ERROR;
    }

    return SERVER_OK;
}

int setnonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags, O_NONBLOCK);
}

tcp_server_status listen_and_accept(int server_fd) {
    struct sockaddr_in client_info = {0};
    socklen_t client_info_len = sizeof(client_info);

    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd;
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        return SERVER_EPOLL_ERROR;
    }

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll ctl; server_fd");
        return SERVER_EPOLL_ERROR;
    }

    while (1) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll ctl; server_fd");
            return SERVER_EPOLL_ERROR;
        }

        for (int n = 0; n < nfds; n++) {
            if (events[n].data.fd == server_fd) {
                int conn_fd = accept(server_fd, (struct sockaddr *)&client_info, &client_info_len);
                if (conn_fd == -1) {
                    perror("accept");
                    return SERVER_ACCEPT_ERROR;
                }

                if (setnonblocking(conn_fd) == -1) {
                    perror("setnonblocking; connfd");
                    return SERVER_ACCEPT_ERROR;
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_fd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
                    perror("epoll ctl; conn_sock");
                    return SERVER_ACCEPT_ERROR;
                }
            } else {
                close(events[n].data.fd);
            }
        }
    }
    return SERVER_OK;
}
