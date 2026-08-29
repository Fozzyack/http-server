#ifndef LOG_H
#define LOG_H

#include <stddef.h>
typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR } log_level;

void log_message(log_level level, const char *format, ...);
void log_errno(log_level level, const char *context);
void log_special_chars(const char *buff, size_t start, size_t end);

#endif // !LOG_H
