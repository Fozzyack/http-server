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

typedef enum {
    CONNECTION_NEW,
    CONNECTION_CONNECTED,
    CONNECTION_DISCONNECTED,
} connection_status;

typedef struct connection {
    int fd;
    int conn_state;

    char buffer[BUFFER_SIZE];
    size_t buffer_start;
    size_t buffer_end;

} connection;

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

int close_connection(int epollfd, connection *conn) {
    close(conn->fd);
    conn->conn_state = CONNECTION_DISCONNECTED;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, conn->fd, NULL);
    return 0;
}

int listen_and_accept(int server_fd) {

    struct sockaddr_in client_info = {0};
    socklen_t client_info_len = sizeof(client_info);
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd, conn_fd = -1;

    connection server_conn = {0};
    server_conn.fd = server_fd;
    server_conn.conn_state = CONNECTION_CONNECTED;
    ev.data.ptr = (connection *)&server_conn;
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
            connection *event_ptr = events[i].data.ptr;
            if (event_ptr && event_ptr->fd == server_fd) {
                conn_fd = accept(server_fd, (struct sockaddr *)&client_info, &client_info_len);
                if (conn_fd == -1) {
                    perror("accept");
                    return -1;
                }
                connection conn = {0};
                conn.fd = conn_fd;
                conn.conn_state = CONNECTION_CONNECTED;
                ev.data.ptr = (connection *)&conn;
                ev.events = EPOLLIN | EPOLLRDHUP; // Need to implement EPOLLET (non blocking)
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
                    perror("epoll_ctl; server_fd");
                    close(conn_fd);
                    return -1;
                }
                // If we add the event then we need to create a new object to hold the state of each file descriptor
                // (connection)
            } else {
                // Parse HTTP request
                // read here
                // after read check for line
                // if \r\n detected - parse line
                // if empty \r\n deteced - we have read the line
                // make sure to read to the buffer inside the pointer we passed to epoll
                // Perform Work
                printf("Huh what is going on\n");
                close_connection(epollfd, event_ptr);
            }
        }
    }

    return -1;
}
