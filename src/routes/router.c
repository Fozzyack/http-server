#include "routes/router.h"

router_status init_route(router *r) {
    r->routes = NULL;
    r->route_count = 0;
    return ROUTER_OK;
}

router_status add_route(char *path, (*handler)(void *args)) { return ROUTER_OK; }

router_status setup_routes(void) { return ROUTER_OK; }
