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
    threadpool *t_pool = (threadpool *)args;
    while (1) {
        pthread_mutex_lock(&t_pool->lock);
        if (t_pool->start == t_pool->end) {
            pthread_cond_wait(&t_pool->condition, &t_pool->lock);
        }

        if (t_pool->pool_stop && t_pool->start == t_pool->end) {
            pthread_mutex_unlock(&t_pool->lock);
            break;
        }
        threadpool_task task = t_pool->task_queue[t_pool->start];
        t_pool->start = t_pool->start + 1 % QUEUE_SIZE;

        pthread_mutex_unlock(&t_pool->lock);
        ((task.fn)(task.args));
    }
    return NULL;
}

threadpool_status threadpool_enqueue_task(void *(*fn)(void *), void *args, threadpool *t_pool) {

    if (t_pool->end == t_pool->start) {
        log_message(LOG_ERROR, "cannot enqueue task queue is full");
        return THREADPOOL_TASK_QUEUE_FULL;
    }

    threadpool_task enqueue_task = {
        .fn = fn,
        .args = args,
    };

    t_pool->task_queue[t_pool->end] = enqueue_task;
    t_pool->end = t_pool->end + 1 % QUEUE_SIZE;

    return THREADPOOL_OK;
}

threadpool_status threadpool_start(threadpool *t_pool) {
    threadpool_status status = threadpool_init(t_pool);
    if (status == THREADPOOL_ERROR) {
        return THREADPOOL_ERROR;
    }
    for (size_t i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&(t_pool->threads[i]), NULL, thread_target, t_pool);
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
