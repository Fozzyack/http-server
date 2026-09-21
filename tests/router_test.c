#include "http/http.h"
#include <assert.h>
#include <routes/router.h>
#include <stdlib.h>

static void test_handler(const http_request *req, http_response *res) {
    (void)req;
    res->status = 204;
}

int main(void) {
    router r = init_router();
    http_request request = {.path = "/test"};
    http_response response = {0};
    assert(add_route(NULL, 0, &r, test_handler) == ROUTER_ERROR);
    assert(add_route("/test", 0, NULL, test_handler) == ROUTER_ERROR);
    assert(add_route("/test", 0, &r, NULL) == ROUTER_ERROR);
    assert(execute_route(NULL, &r, &response) == ROUTER_ROUTE_ERROR);
    assert(execute_route(&request, NULL, &response) == ROUTER_ROUTE_ERROR);
    assert(execute_route(&request, &r, NULL) == ROUTER_ROUTE_ERROR);

    assert(add_route("/test", 0, &r, test_handler) == ROUTER_OK);
    assert(execute_route(&request, &r, &response) == ROUTER_ROUTE_FOUND);
    assert(response.status == 204);

    delete_router(&r);

    return EXIT_SUCCESS;
}
