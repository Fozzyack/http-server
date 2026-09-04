#include "server/listener.h"
#include "server/server.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 100

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
