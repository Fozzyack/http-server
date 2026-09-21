#include "http/http.h"
#include "log/log.h"
#include <routes/router.h>

void healthcheck(const http_request *req, const http_response *res) {

    log_message(LOG_INFO, "Server is healthy %s", req->path);
    return;
}
