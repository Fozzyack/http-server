#include "threadpool/threadpool.h"
#include "log/log.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

threadpool_status threadpool_init(threadpool *t_pool) {
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

threadpool_status threadpool_enqueue_task(void *(*fn)(void *), void *args, threadpool *t_pool) {

    if (t_pool->end == t_pool->start) {
        log_message(LOG_ERROR, "cannot enqueue task queue is full");
        return THREADPOOL_TASK_QUEUE_FULL;
    }

    size_t idx = (t_pool->end + 1) % QUEUE_SIZE;

    task enqueue_task = {
        .fn = fn,
        .args = args,
    };

    t_pool->task_queue[t_pool->end] = enqueue_task;

    return THREADPOOL_OK;
}

threadpool_status threadpool_start(threadpool *t_pool) {
    threadpool_status status = threadpool_init(t_pool);
    if (status == THREADPOOL_ERROR) {
        return THREADPOOL_ERROR;
    }
    for (size_t i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&(t_pool->threads[i]), NULL, thread_target, NULL);
    }
    return THREADPOOL_OK;
}

threadpool_status threadpool_stop(threadpool *t_pool) {
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(t_pool->threads[i], NULL);
    }
    pthread_mutex_destroy(&(t_pool->lock));
    pthread_cond_destroy(&(t_pool->condition));
    return THREADPOOL_OK;
}
