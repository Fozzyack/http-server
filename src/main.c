#include "http/parser.h"
#include "http/response.h"
#include "log/log.h"
#include "server/server.h"
#include "threadpool/threadpool.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handle_client(int client_fd) {
    http_request req = {0};
    int status;
    if ((status = parse_http_request(&req, client_fd)) != PARSE_OK) {
        log_message(LOG_ERROR, "huh %d", status);
        return;
    }
}

void test_http_res(int client_fd) {
    http_response res = {0};
    init_response(&res);
    response_set_header("Content-Type", "application/json", &res);
    response_set_header("Content-Length", "32", &res);
    response_set_header("Connection", "keep-alive", &res);
    const char *json = "{\"message\":\"hello\",\"what\":{\"is\":\"going on\"}}";
    response_set_json(json, &res);
    send_response(client_fd, &res);
    destroy_response(&res);
}

void test_thread_fn(void *args) {
    sleep(1);
    threadpool *t_pool = (threadpool *)args;
    fprintf(stderr, "Stop flag - %d\n", t_pool->pool_stop);
    fprintf(stderr, "queue size %zu\n", t_pool->queue_count);
    return;
}

int main(void) {

    tcp_server_info server_info = {0};
    tcp_server_status server_status = bind_tcp_server(&server_info, 8080);
    if (server_status != SERVER_OK) {
        return EXIT_FAILURE;
    }

    // while (1) {
    //     int client_fd;
    //     server_status = accept_client(server_info.socket_fd, &client_fd);
    //     if (server_status == SERVER_ACCEPT_ERROR) {
    //         log_message(LOG_ERROR, "Failed to accept client");
    //         return EXIT_FAILURE;
    //     }
    //     test_http_res(client_fd);
    //     close(client_fd);
    // }
    threadpool t_pool = {0};
    threadpool_start(&t_pool);
    sleep(5);
    for (int i = 0; i < 20; i++) {
        threadpool_enqueue_task(test_thread_fn, &t_pool, &t_pool);
    }
    sleep(21);
    threadpool_stop(&t_pool);
    close(server_info.socket_fd);
    return EXIT_SUCCESS;
}
