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

typedef enum status_code {
    HTTP_OK,
    HTTP_NOT_FOUND,
} status_code;

typedef struct route_handler {
    void (*fn)(void);
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
void healthcheck(void);

#endif // !ROUTER_H
