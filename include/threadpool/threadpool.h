#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stddef.h>

#define THREAD_COUNT 10
#define QUEUE_SIZE 100

typedef enum {
    THREADPOOL_OK,
    THREADPOOL_ERROR,
} threadpool_status;

typedef struct task {
    void (*fn)(void *);
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

threadpool_status start_threadpool(threadpool *t_pool);

#endif // !THREADPOOL_H
