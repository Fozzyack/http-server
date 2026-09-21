#include "threadpool/threadpool.h"
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define TASK_COUNT 20

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t complete;
    size_t completed;
} test_state;

static void count_task(void *args) {
    test_state *state = args;

    pthread_mutex_lock(&state->lock);
    state->completed++;
    pthread_cond_signal(&state->complete);
    pthread_mutex_unlock(&state->lock);
}

int main(void) {
    threadpool pool = {0};
    test_state state = {
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .complete = PTHREAD_COND_INITIALIZER,
    };
    struct timespec deadline;

    assert(threadpool_start(NULL) == THREADPOOL_ERROR);
    assert(threadpool_start(&pool) == THREADPOOL_OK);
    assert(threadpool_enqueue_task(NULL, NULL, &pool) == THREADPOOL_ERROR);
    assert(threadpool_enqueue_task(count_task, &state, NULL) == THREADPOOL_ERROR);

    for (size_t i = 0; i < TASK_COUNT; i++) {
        assert(threadpool_enqueue_task(count_task, &state, &pool) == THREADPOOL_OK);
    }

    assert(clock_gettime(CLOCK_REALTIME, &deadline) == 0);
    deadline.tv_sec += 5;

    pthread_mutex_lock(&state.lock);
    while (state.completed < TASK_COUNT) {
        assert(pthread_cond_timedwait(&state.complete, &state.lock, &deadline) == 0);
    }
    pthread_mutex_unlock(&state.lock);

    assert(threadpool_stop(&pool) == THREADPOOL_OK);
    pthread_mutex_destroy(&state.lock);
    pthread_cond_destroy(&state.complete);
    return EXIT_SUCCESS;
}
