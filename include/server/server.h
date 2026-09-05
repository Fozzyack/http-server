#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>

typedef enum {
    SERVER_OK,
    SERVER_ERROR,
    SERVER_INFO_DOES_NOT_EXIST,
    SERVER_SOCKET_INIT_ERROR,
    SERVER_SETSOCKOPT_ERROR,
    SERVER_BIND_ERROR,
    SERVER_ACCEPT_ERROR,
    SERVER_EPOLL_ERROR,
} tcp_server_status;

typedef struct {
    int socket_fd;
    struct sockaddr_in address;
} tcp_server_info;

tcp_server_status bind_tcp_server(tcp_server_info *server_info, int port);
int listen_and_accept(int server_fd);

#endif // !SERVER_H
