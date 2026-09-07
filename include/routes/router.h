#ifndef ROUTER_H
#define ROUTER_H

#include <stddef.h>

#define REQUEST_TARGET_LENGTH 2048 // copy of the one in http.h (will need to restructure to use a shared macro)

typedef enum router_status {
    ROUTER_OK,
    ROUTER_ERROR,
    ROUTER_ADD_ROUTE_ERROR,
    ROUTER_ADD_INVALID_THREADED_VALUE,
    ROUTER_ADD_INVALID_PATH_TOO_LONG,
} router_status;

typedef struct route_handler {
    void (*fn)(void *args);
    void *args;
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

router_status add_route(char *path, int is_threaded, router *r, void (*handler)(void *args), void *args);
router_status setup_routes(router *r);
void delete_router(router *r);

#endif // !ROUTER_H
