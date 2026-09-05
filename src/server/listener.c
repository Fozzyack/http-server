#include "http/http.h"
#include "log/log.h"
#include "server/server.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 1000

int handle_http_request(int client_id) {
    http_request req = {0};
    if (parse_http_request(&req, client_id) != PARSE_OK) {
        log_message(LOG_ERROR, "Failed to parse http request");
        return -1;
    }

    log_message(LOG_INFO, "%s %s %s", req.method, req.path, req.protocol);
    for (size_t i = 0; i < req.header_count; i++) {
        log_message(LOG_INFO, "%s: %s", req.headers[i].name, req.headers[i].value);
    }

    return 0;
}

int setnonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags, O_NONBLOCK);
}

int listen_and_accept(int server_fd) {

    struct sockaddr_in client_info = {0};
    socklen_t client_info_len = sizeof(client_info);
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd, conn_fd = -1;
    ev.data.fd = server_fd;
    ev.events = EPOLLIN;

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create");
        return -1;
    }

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl; server_fd");
        return -1;
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll wait");
            return -1;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                conn_fd = accept(server_fd, (struct sockaddr *)&client_info, &client_info_len);
                if (conn_fd == -1) {
                    perror("accept");
                    return -1;
                }
                ev.data.fd = conn_fd;
                ev.events = EPOLLIN | EPOLLRDHUP; // Need to implement EPOLLET (non blocking)
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
                    perror("epoll_ctl; server_fd");
                    close(conn_fd);
                    return -1;
                }

            } else {
                log_message(LOG_INFO, "fd waiting for read/write");
                close(conn_fd);
            }
        }
    }

    return -1;
}
