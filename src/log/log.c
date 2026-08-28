#include "log/log.h"
#include <stdarg.h>
#include <stdio.h>

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

    return;
}
