#include "routes/router.h"
#include "log/log.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

router init_router(void) {
    router r;
    r.routes = NULL;
    r.route_count = 0;
    return r;
}

void delete_router(router *r) { free(r->routes); }

router_status add_route(const char *path, int is_threaded, router *r,
                        void (*handler)(const http_request *, http_response *)) {

    if (path == NULL || r == NULL || handler == NULL) {
        log_message(LOG_ERROR, "add_route: path, router, and handler must not be NULL");
        return ROUTER_ERROR;
    }

    if (is_threaded > 1 || is_threaded < 0) {
        log_message(LOG_ERROR, "invalid thread value given");
        return ROUTER_ADD_INVALID_THREADED_VALUE;
    }

    size_t path_size = strlen(path);
    if (path_size >= REQUEST_TARGET_LENGTH) {
        log_message(LOG_ERROR, "invalid path; length too long");
        return ROUTER_ADD_INVALID_PATH_TOO_LONG;
    }

    if (r->route_count == 0) {
        r->routes = calloc(1, sizeof(route));
        if (r->routes == NULL) {
            log_errno(LOG_ERROR, "calloc; add_route");
            return ROUTER_ADD_ROUTE_ERROR;
        }
    } else {
        route *temp_routes = realloc(r->routes, sizeof(route) * (r->route_count + 1));
        if (temp_routes == NULL) {
            log_errno(LOG_ERROR, "realloc; add_route");
            return ROUTER_ADD_ROUTE_ERROR;
        }
        r->routes = temp_routes;
    }

    memcpy(r->routes[r->route_count].path.name, path, path_size);
    r->routes[r->route_count].path.name[path_size] = '\0';
    r->routes[r->route_count].path.size = path_size;
    r->routes[r->route_count].handler.fn = handler;
    r->routes[r->route_count].is_threaded = is_threaded;
    r->route_count++;
    return ROUTER_OK;
}

route_result execute_route(const http_request *req, const router *r, http_response *res) {

    if (req == NULL || r == NULL || res == NULL) {
        log_message(LOG_ERROR, "execute_route: request, router, and response must not be NULL");
        return ROUTER_ROUTE_ERROR;
    }

    for (size_t i = 0; i < r->route_count; i++) {

        if (!strcmp(req->path, r->routes[i].path.name)) {
            r->routes[i].handler.fn(req, res);
            return ROUTER_ROUTE_FOUND;
        }
    }
    // If  we reach here return 404
    return ROUTER_ROUTE_NOT_FOUND;
}

router_status setup_router(router *r) {
    *r = init_router();
    router_status status = add_route("/healthcheck", 0, r, healthcheck);
    if (status != ROUTER_OK) {
        log_message(LOG_ERROR, "add router; healthcheck");
        return status;
    }
    return ROUTER_OK;
}
