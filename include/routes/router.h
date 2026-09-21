#ifndef ROUTER_H
#define ROUTER_H

#include "http/http.h"
#include <stddef.h>

typedef enum router_status {
    ROUTER_OK,
    ROUTER_ERROR,
    ROUTER_ADD_ROUTE_ERROR,
    ROUTER_ADD_INVALID_THREADED_VALUE,
    ROUTER_ADD_INVALID_PATH_TOO_LONG,
} router_status;

typedef enum route_result {
    ROUTE_FOUND,
    ROUTE_NOT_FOUND,
} route_result;

typedef struct route_handler {
    void (*fn)(const http_request *request, const http_response *http_response);
    int client_fd;
} route_handler;

typedef struct route_path {
    char name[REQUEST_TARGET_LENGTH];
    size_t size;
} route_path;

typedef struct route {
    route_path path;
    route_handler handler;
    int is_threaded;
} route;

typedef struct router {
    route *routes;
    size_t route_count;
} router;

// router setup
router_status setup_router(router *r);
void delete_router(router *r);

// routes
void healthcheck(const http_request *, const http_response *);
router init_router(void);
router_status add_route(char *path, int is_threaded, router *r, void (*handler)(int client_fd));
route_result execute_route(char *path, int client_fd, router *r);

#endif // !ROUTER_H
