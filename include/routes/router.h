#ifndef ROUTER_H
#define ROUTER_H

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

#endif // !ROUTER_H
