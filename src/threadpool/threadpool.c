#include "threadpool/threadpool.h"
#include "log/log.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

threadpool_status init_threadpool(threadpool *t_pool) {
    pthread_mutex_init(&(t_pool->lock), NULL);
    int status = pthread_cond_init(&(t_pool->condition), NULL);
    if (status != 0) {
        log_message(LOG_ERROR, "init_threadpool: pthread_cond_init failed");
        return THREADPOOL_ERROR;
    }
    t_pool->pool_stop = 0;
    t_pool->end = 0;
    t_pool->start = 0;

    return THREADPOOL_OK;
}

void *thread_target(void *args) {
    (void)args;
    fprintf(stderr, "Thread running\n");
    return NULL;
}

threadpool_status start_threadpool(threadpool *t_pool) {
    threadpool_status status = init_threadpool(t_pool);
    if (status == THREADPOOL_ERROR) {
        return THREADPOOL_ERROR;
    }
    for (size_t i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&(t_pool->threads[i]), NULL, thread_target, NULL);
    }
    return THREADPOOL_OK;
}
