#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>

typedef enum {
    SERVER_OK,
    SERVER_ERROR,
} tcp_server_status;

typedef struct {
    int socket_fd;
    struct sockaddr_in address;
} tcp_server_info;

#endif // !SERVER_H
