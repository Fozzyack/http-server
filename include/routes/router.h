#ifndef ROUTER_H
#define ROUTER_H

#include <stddef.h>

typedef enum router_status {
    ROUTER_OK,
    ROUTER_ERROR,
} router_status;

typedef struct route_handler {
    void (*fn)(void *args);
    void *args;
} route_handler;

typedef struct route {
    char *path;
    route_handler handler;
} route;

typedef struct router {
    route *routes;
    size_t route_count;
} router;

#endif // !ROUTER_H
