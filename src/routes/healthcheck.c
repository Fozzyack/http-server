#include "http/http.h"
#include "log/log.h"
#include <routes/router.h>

void healthcheck(const http_request *req, http_response *res) {
    (void)res;
    log_message(LOG_INFO, "Server is healthy %s", req->path);
    return;
}
