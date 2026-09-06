#include "http/http.h"
#include "log/log.h"
#include "server/server.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 1000

typedef enum {
    NEW_CONNECTION,
    PARSED_REQUEST_LINE,
    PARSED_HEADERS,
    PARSED_BODY,
    WRITTEN_RESPONSE,
} connection_status;

typedef struct connection {
    int fd;
    int conn_state;

    http_request request;
    http_request_buffer buffer;
} connection;

int setnonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int close_connection(int epollfd, connection *conn) {
    close(conn->fd);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, conn->fd, NULL);
    free(conn);
    return 0;
}

int listen_and_accept(int server_fd) {

    struct sockaddr_in client_info = {0};
    socklen_t client_info_len = sizeof(client_info);
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd, conn_fd = -1;

    connection server_conn = {0};
    server_conn.fd = server_fd;
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
                connection *conn = malloc(sizeof(connection));
                if (conn == NULL) {
                    log_errno(LOG_ERROR, "malloc; could not create new conn");
                    return -1;
                }
                conn->fd = conn_fd;
                conn->conn_state = NEW_CONNECTION;
                init_request_info(&(conn->request), &(conn->buffer));
                if (setnonblocking(conn_fd) == -1) {
                    perror("setnonblocking");
                    close(conn_fd);
                    free(conn);
                    continue;
                }

                ev.data.ptr = (connection *)conn;
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
                parse_status status = -1;
                int conn_open = 1;
                for (;;) {
                    size_t eol = 0;
                    if (event_ptr->conn_state == NEW_CONNECTION) {
                        status = read_from_socket(event_ptr->fd, &event_ptr->buffer);
                        if (status == PARSE_READ_ERROR) {
                            log_message(LOG_ERROR, "Failed to Read Response\n");
                            close_connection(epollfd, event_ptr);
                            conn_open = 0;
                            break;
                        }
                        status = find_line(&event_ptr->buffer, &eol);
                        if (status != PARSE_OK) {
                            break;
                        }
                        status = parse_request_line(&event_ptr->request, &event_ptr->buffer, &eol);
                        if (status != PARSE_OK) {
                            log_message(LOG_ERROR, "Failed to Parse Request Line\n");
                            conn_open = 0;
                            close_connection(epollfd, event_ptr);
                            break;
                        } else {
                            event_ptr->conn_state = PARSED_REQUEST_LINE;
                        }
                    }
                    if (event_ptr->conn_state == PARSED_REQUEST_LINE) {
                        status = read_from_socket(event_ptr->fd, &event_ptr->buffer);
                        if (status == PARSE_READ_ERROR) {
                            conn_open = 0;
                            close_connection(epollfd, event_ptr);
                            break;
                        }

                        while (event_ptr->conn_state != PARSED_HEADERS) {
                            status = find_line(&event_ptr->buffer, &eol);
                            if (status != PARSE_OK) {
                                break;
                            }
                            if (eol == 2) {
                                event_ptr->conn_state = PARSED_HEADERS;
                                break;
                            }
                            status = parse_header(&event_ptr->request, &event_ptr->buffer, &eol);
                            if (status != PARSE_OK) {
                                conn_open = 0;
                                log_message(LOG_ERROR, "Failed to Parse Header\n");
                                close_connection(epollfd, event_ptr);
                                break;
                            }
                        }
                        break;
                    }
                }

                if (!conn_open) {
                    continue;
                }

                if (event_ptr->conn_state != PARSED_HEADERS) {
                    continue;
                }

                log_message(LOG_INFO, "%s %s %s", event_ptr->request.method, event_ptr->request.path,
                            event_ptr->request.protocol);
                for (size_t i = 0; i < event_ptr->request.header_count; i++) {
                    log_message(LOG_INFO, "%s: %s", event_ptr->request.headers[i].name,
                                event_ptr->request.headers[i].value);
                }
                close_connection(epollfd, event_ptr);
            }
        }
    }

    return -1;
}
