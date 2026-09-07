#include "log/log.h"
#include <routes/router.h>

void healthcheck(int client_fd) {
    log_message(LOG_INFO, "Server is healthy");
    return;
}
