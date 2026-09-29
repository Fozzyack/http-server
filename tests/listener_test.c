#include "routes/router.h"
#include "server/server.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

static int connect_to_server(int port) {
    struct sockaddr_in address = {0};
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd != -1);

    address.sin_family = AF_INET;
    address.sin_port = htons((uint16_t)port);
    assert(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) == 1);
    assert(connect(fd, (struct sockaddr *)&address, sizeof(address)) == 0);

    return fd;
}

static void send_all(int fd, const char *data, size_t length) {
    size_t sent = 0;
    while (sent < length) {
        ssize_t result = send(fd, data + sent, length - sent, 0);
        assert(result > 0);
        sent += (size_t)result;
    }
}

static size_t read_response(int fd, char *buffer, size_t capacity) {
    size_t total = 0;
    while (total < capacity - 1) {
        ssize_t result = read(fd, buffer + total, capacity - 1 - total);
        if (result == -1) {
            assert(errno == EINTR);
            continue;
        }
        if (result == 0) {
            break;
        }
        total += (size_t)result;
    }
    buffer[total] = '\0';
    return total;
}

static void test_healthcheck(int port) {
    char response[512];
    const char request[] = "GET /healthcheck HTTP/1.1\r\nHost: localhost\r\n\r\n";
    int fd = connect_to_server(port);

    send_all(fd, request, sizeof(request) - 1);
    assert(read_response(fd, response, sizeof(response)) > 0);
    assert(strstr(response, "HTTP/1.1 200 OK\r\n") != NULL);
    assert(strstr(response, "\r\n\r\n{\"status\":\"ok\"}") != NULL);

    close(fd);
}

int main(void) {
    tcp_server_info server_info = {0};
    router router_info = {0};
    struct sockaddr_in bound_address = {0};
    socklen_t bound_length = sizeof(bound_address);
    int port;

    assert(init_server(&server_info, 0) == SERVER_OK);
    assert(bind_tcp_server(&server_info) == SERVER_OK);
    assert(getsockname(server_info.socket_fd, (struct sockaddr *)&bound_address, &bound_length) == 0);
    port = ntohs(bound_address.sin_port);
    setup_router(&router_info);

    pid_t server_pid = fork();
    assert(server_pid >= 0);
    if (server_pid == 0) {
        listen_and_accept(server_info.socket_fd, &router_info);
        _exit(EXIT_SUCCESS);
    }

    close(server_info.socket_fd);

    test_healthcheck(port);

    assert(kill(server_pid, SIGKILL) == 0);
    assert(waitpid(server_pid, NULL, 0) == server_pid);

    return EXIT_SUCCESS;
}
