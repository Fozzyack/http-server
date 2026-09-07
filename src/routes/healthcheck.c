#include "log/log.h"
#include <routes/router.h>

void healthcheck(void) {
    log_message(LOG_INFO, "Server is healthy");
    return;
}
