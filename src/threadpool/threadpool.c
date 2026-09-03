#include "threadpool/threadpool.h"
#include "log/log.h"
#include <pthread.h>

threadpool_status init_threadpool(threadpool *t_pool) {
    pthread_mutex_init(&(t_pool->lock), NULL);
    int status = pthread_cond_init(&(t_pool->condition), NULL);
    if (status != 0) {
        log_message(LOG_ERROR, "pthread_cond_init failed");
        return THREADPOOL_ERROR;
    }
    t_pool->pool_stop = 0;
    t_pool->end = 0;
    t_pool->start = 0;

    return THREADPOOL_OK;
}
