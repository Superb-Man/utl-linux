#include "cond.h"
#include "uthread.h"

void uthread_cond_init(uthread_cond_t* cond) {
    queue_init(&cond->waiters);
    uthread_mutex_init(&cond->lock);
}

void uthread_cond_wait(uthread_cond_t* cond, uthread_mutex_t* mutex) {
    uthread_tcb_t* current = &thread_table[get_tid()];
    current->state = THREAD_BLOCKED;
    queue_push(&cond->waiters, current); // add current into waiting queue
    uthread_mutex_unlock(mutex);
    uthread_yield();
    uthread_mutex_lock(mutex);
}

void uthread_cond_signal(uthread_cond_t* cond) {
    uthread_mutex_lock(&cond->lock);
    if (!queue_is_empty(&cond->waiters)) {
        uthread_tcb_t* tcb = queue_pop(&cond->waiters);
        tcb->state = THREAD_READY;
        enqueue_thread(tcb); // add the thread back to the ready queue
    }
    uthread_mutex_unlock(&cond->lock);
}


void uthread_cond_broadcast(uthread_cond_t* cond) {
    uthread_mutex_lock(&cond->lock); // might not be necessary for single thread
    while (!queue_is_empty(&cond->waiters)) {
        uthread_cond_signal(cond);
    }
    uthread_mutex_unlock(&cond->lock);
}
