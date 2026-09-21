#include "log/log.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    int pipe_fds[2];
    int saved_stderr;
    char output[256] = {0};
    char expected[256] = {0};

    assert(pipe(pipe_fds) == 0);
    saved_stderr = dup(STDERR_FILENO);
    assert(saved_stderr != -1);
    assert(dup2(pipe_fds[1], STDERR_FILENO) == STDERR_FILENO);
    close(pipe_fds[1]);

    log_message(LOG_INFO, "value %d", 42);
    errno = ENOENT;
    log_errno(LOG_ERROR, "open");
    log_special_chars("xa\r\nb", 1, 5);
    fflush(stderr);

    assert(dup2(saved_stderr, STDERR_FILENO) == STDERR_FILENO);
    close(saved_stderr);
    snprintf(expected, sizeof(expected), "[INFO] value 42\n[ERROR] open: %s\n[DEBUG] a\\r\\nb\n", strerror(ENOENT));
    assert(read(pipe_fds[0], output, sizeof(output)) == (ssize_t)strlen(expected));
    close(pipe_fds[0]);
    assert(memcmp(output, expected, strlen(expected)) == 0);

    return EXIT_SUCCESS;
}
