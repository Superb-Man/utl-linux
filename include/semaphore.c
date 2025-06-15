#include "semaphore.h"
#include "uthread.h"
#include "queue.h"

void uthread_sem_init(uthread_sem_t* sem, int value) {
    sem->value = value;
    queue_init(&sem->waiters);
}


void uthread_sem_wait(uthread_sem_t* sem) {
    if (sem->value > 0) {
        sem->value--;
    } 
    else {
        uthread_tcb_t* current = &thread_table[get_tid()];
        current->state = THREAD_BLOCKED;
        queue_push(&sem->waiters, current);
        uthread_yield();
    }
}


void uthread_sem_post(uthread_sem_t* sem) {
    if (!queue_is_empty(&sem->waiters)) {
        uthread_tcb_t* tcb = (uthread_tcb_t*)queue_pop(&sem->waiters);
        tcb->state = THREAD_READY;
        enqueue_thread(tcb);
    } 
    else {
        sem->value++;
    }
}
