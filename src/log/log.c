#include "log/log.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void log_message(log_level level, const char *format, ...) {
    va_list args;

    switch (level) {

    case LOG_DEBUG:
        fputs("[DEBUG] ", stderr);
        break;
    case LOG_INFO:
        fputs("[INFO] ", stderr);
        break;
    case LOG_WARN:
        fputs("[WARN] ", stderr);
        break;
    case LOG_ERROR:
        fputs("[ERROR] ", stderr);
        break;
    default:
        fputs("[UNKNOWN] ", stderr);
    }

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);

    return;
}

void log_errno(log_level level, const char *context) {
    int error = errno;

    log_message(level, "%s: %s", context, strerror(error));
}
