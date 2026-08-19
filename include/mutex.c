#include "mutex.h"
#include "uthread.h"
#include "queue.h"

void uthread_mutex_init(uthread_mutex_t* mutex) {
    mutex->locked = 0;
    queue_init(&mutex->waiters);
}


void uthread_mutex_lock(uthread_mutex_t* mutex) {
    block();   // prevent SIGALRM preemption during the critical section below

    // Fast path: acquire immediately if free
    if (!__sync_lock_test_and_set(&mutex->locked, 1)) {
        unblock();
        return;
    }

    // Slow path: push ourselves EXACTLY ONCE, then hand control to the scheduler.
    uthread_tcb_t* current = &thread_table[get_tid()];
    current->state = THREAD_BLOCKED;
    queue_push(&mutex->waiters, current);
    unblock();
    uthread_yield();

    // We are woken ONLY by unlock() handing the lock to us directly --
    // no re-check loop, no second push. We return as the new owner.
}

void uthread_mutex_unlock(uthread_mutex_t* mutex) {
    block();   // atomic with respect to the wake-up

    if (!queue_is_empty(&mutex->waiters)) {
        // Hand the lock to the next waiter WITHOUT ever setting locked = 0.
        uthread_tcb_t* next = queue_pop(&mutex->waiters);
        next->state = THREAD_READY;
        enqueue_thread(next);
        // `mutex->locked` intentionally stays 1: ownership has moved to `next`.
        // Any other thread trying to lock in the meantime will block and enqueue.
    } else {
        // No waiters: release so the fast path can acquire it.
        __sync_lock_release(&mutex->locked);   // matches the test_and_set in lock()
    }
    unblock();
}

