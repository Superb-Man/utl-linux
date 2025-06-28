#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "uthread.h"
#include "queue.h"

static queue_t ready_queue;
extern uthread_tcb_t thread_table[MAX_THREADS];
extern uthread_t current_tid;
static int scheduler_initialized = 0;

void scheduler_init(void) {
    if (!scheduler_initialized) {
        queue_init(&ready_queue);
        scheduler_initialized = 1;
    }
}

void enqueue_thread(uthread_tcb_t *tcb) {
    if (tcb->state == THREAD_READY && tcb->tid != 0) {
        queue_push(&ready_queue, tcb);
        DEBUG_PRINT("Queue size after enqueue: %d\n", queue_size(&ready_queue));
    }
}


void schedule_next(void) {
    uthread_tcb_t* prev = &thread_table[current_tid];
    uthread_tcb_t* next = NULL;

    while (!queue_is_empty(&ready_queue)) {
        next = (uthread_tcb_t* ) queue_pop(&ready_queue);
        if (next->state == THREAD_READY)
            break;
        next = NULL;
    }

    DEBUG_PRINT("[scheduler] Switching from thread %d to thread %d\n", current_tid, next ? next->tid : -1);


    if (!next) {
        // Check if any threads are still BLOCKED or RUNNING
        int any_active = 0;
        for (int i = 1; i < MAX_THREADS; i++) {
            if (thread_table[i].state == THREAD_BLOCKED || thread_table[i].state == THREAD_RUNNING) {
                any_active = 1;
                break;
            }
        }

        if (!any_active) {
            ERROR_PRINT("[scheduler] All threads have finished.\n");
            exit(0);
        }

        return; 
    }

    current_tid = next->tid;
    next->state = THREAD_RUNNING;

    DEBUG_PRINT("Switching from thread %d to thread %d\n", prev->tid, next->tid);
    DEBUG_PRINT("address of prev->context: %p\n", (void*)&prev->context);
    DEBUG_PRINT("address of next->context: %p\n", (void*)&next->context);


    swapcontext(&prev->context, &next->context);
}