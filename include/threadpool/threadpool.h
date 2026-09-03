#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stddef.h>

#define THREAD_COUNT 10
#define QUEUE_SIZE 100

typedef enum {
    THREADPOOL_OK,
    THREADPOOL_ERROR,
    THREADPOOL_TASK_QUEUE_FULL,
} threadpool_status;

typedef struct task {
    void *(*fn)(void *);
    void *args;
} task;

typedef struct threadpool {

    pthread_mutex_t lock;
    pthread_cond_t condition;
    pthread_t threads[THREAD_COUNT];
    int pool_stop;

    task task_queue[QUEUE_SIZE];
    size_t start;
    size_t end;

} threadpool;

threadpool_status threadpool_start(threadpool *t_pool);
threadpool_status threadpool_stop(threadpool *t_pool);
threadpool_status threadpool_enqueue_task(void *(*fn)(void *), void *args, threadpool *t_pool);

#endif // !THREADPOOL_H
