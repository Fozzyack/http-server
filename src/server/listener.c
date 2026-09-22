#include "http/http.h"
#include "log/log.h"
#include "routes/router.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_EVENTS 1000

typedef enum {
    NEW_CONNECTION,
    PARSED_REQUEST_LINE,
    PARSED_HEADERS,
    PARSED_BODY,
    ROUTE_FOUND,
    ROUTE_NOT_FOUND,
    WRITTEN_RESPONSE,
} connection_status;

typedef struct connection {
    int fd;
    int conn_state;

    http_request request;
    http_request_buffer buffer;

    char *response_data;
    size_t response_length;
    size_t response_sent;

    route conn_route;
} connection;

int setnonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int close_connection(int epollfd, connection *conn) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, conn->fd, NULL);
    close(conn->fd);
    free(conn->response_data);
    free(conn);
    return 0;
}

int listen_and_accept(int server_fd, router *r) {

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
                conn->response_data = NULL;
                conn->response_length = 0;
                conn->response_sent = 0;
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
                parse_status status = -1;
                int conn_open = 1;
                for (;;) {
                    size_t eol = 0;
                    // Read incoming bytes until a complete request line is available.
                    // Parse it and advance the connection to header parsing.
                    if (event_ptr->conn_state == NEW_CONNECTION) {
                        status = read_from_socket(event_ptr->fd, &event_ptr->buffer);
                        if (status == PARSE_READ_ERROR || status == PARSE_READ_SOCKET_DISCONNECTED) {
                            log_message(LOG_ERROR, "Failed to read request");
                            close_connection(epollfd, event_ptr);
                            break;
                        }
                        status = find_line(&event_ptr->buffer, &eol);
                        if (status != PARSE_OK) {
                            break;
                        }
                        status = parse_request_line(&event_ptr->request, &event_ptr->buffer, &eol);
                        if (status != PARSE_OK) {
                            log_message(LOG_ERROR, "Failed to parse request line");
                            close_connection(epollfd, event_ptr);
                            break;
                        }
                        event_ptr->conn_state = PARSED_REQUEST_LINE;
                        // Consume complete header lines from the buffered request.
                        // A blank line finishes the header section and advances the connection.
                    } else if (event_ptr->conn_state == PARSED_REQUEST_LINE) {
                        status = read_from_socket(event_ptr->fd, &event_ptr->buffer);
                        if (status == PARSE_READ_ERROR || status == PARSE_READ_SOCKET_DISCONNECTED) {
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
                                log_message(LOG_ERROR, "Failed to parse header");
                                close_connection(epollfd, event_ptr);
                                break;
                            }
                        }
                        if (!conn_open || event_ptr->conn_state != PARSED_HEADERS) {
                            break;
                        }
                        // Route the complete request and serialize the resulting response.
                        // Switch epoll to write readiness, then yield until EPOLLOUT arrives.
                    } else if (event_ptr->conn_state == PARSED_HEADERS) {
                        http_response res;
                        init_response(&res);

                        route_result result = execute_route(&event_ptr->request, r, &res);
                        if (result == ROUTER_ROUTE_NOT_FOUND) {
                            res.status = 404;
                            strcpy(res.status_response, "Not Found");
                            response_set_json("{\"error\":\"Not Found\"}", &res);
                        }

                        event_ptr->response_data = construct_response(&res, &event_ptr->response_length);
                        event_ptr->response_sent = 0;
                        destroy_response(&res);
                        if (event_ptr->response_data == NULL) {
                            log_message(LOG_ERROR, "Failed to construct response");
                            close_connection(epollfd, event_ptr);
                            break;
                        }

                        event_ptr->conn_state = WRITTEN_RESPONSE;
                        ev.data.ptr = event_ptr;
                        ev.events = EPOLLOUT | EPOLLRDHUP;
                        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, event_ptr->fd, &ev) == -1) {
                            log_errno(LOG_ERROR, "epoll_ctl; response write");
                            close_connection(epollfd, event_ptr);
                        }
                        break;
                        // Write available response bytes without blocking the event loop.
                        // Keep the connection until every byte is sent or a write error occurs.
                    } else if (event_ptr->conn_state == WRITTEN_RESPONSE) {
                        if ((events[i].events & EPOLLOUT) == 0) {
                            break;
                        }

                        ssize_t bytes_sent = send(event_ptr->fd, event_ptr->response_data + event_ptr->response_sent,
                                                  event_ptr->response_length - event_ptr->response_sent, 0);
                        if (bytes_sent > 0) {
                            event_ptr->response_sent += (size_t)bytes_sent;
                            if (event_ptr->response_sent == event_ptr->response_length) {
                                log_message(LOG_INFO, "%s %s %s", event_ptr->request.method, event_ptr->request.path,
                                            event_ptr->request.protocol);
                                close_connection(epollfd, event_ptr);
                            }
                        } else if (bytes_sent == 0) {
                            log_message(LOG_ERROR, "send; response write made no progress");
                            close_connection(epollfd, event_ptr);
                        } else if (bytes_sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                            log_errno(LOG_ERROR, "send; response write");
                            close_connection(epollfd, event_ptr);
                        }
                        break;
                    }
                }
            }
        }
    }

    return -1;
}
