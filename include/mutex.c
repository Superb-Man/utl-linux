#include "mutex.h"
#include "uthread.h"
#include "queue.h"

void uthread_mutex_init(uthread_mutex_t* mutex) {
    mutex->locked = 0;
    queue_init(&mutex->waiters);
}


void uthread_mutex_lock(uthread_mutex_t* mutex) {
    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        uthread_tcb_t *current = &thread_table[get_tid()];
        current->state = THREAD_BLOCKED;
        queue_push(&mutex->waiters, current);
        uthread_yield();
    }
}

void uthread_mutex_unlock(uthread_mutex_t* mutex) {
    mutex->locked = 0;
    if (!queue_is_empty(&mutex->waiters)) {
        uthread_tcb_t* next = queue_pop(&mutex->waiters);
        next->state = THREAD_READY;
        enqueue_thread(next);
    }
}
