#ifndef LOG_H
#define LOG_H

typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR } log_level;

void log_message(log_level level, const char *format, ...);
void log_errno(log_level level, const char *context);

#endif // !LOG_H
