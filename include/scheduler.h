#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "uthread.h"
#include "queue.h"

static queue_t ready_queue;
extern uthread_tcb_t thread_table[MAX_THREADS];
extern uthread_t current_tid;
// static int scheduler_initialized = 0;

// void scheduler_init(void) {
//     if (!scheduler_initialized) {
//         queue_init(&ready_queue);
//         scheduler_initialized = 1;
//     }
// }

void enqueue_thread(uthread_tcb_t* tcb) {
    if (tcb->state == THREAD_READY) {
        queue_push(&ready_queue, tcb);
        DEBUG_PRINT("[scheduler] Enqueued thread %d\n", tcb->tid);

    }
}

/**
 * @brief Schedule the next thread to run
 * 
 * @details This function checks the ready queue for the next thread to run.
 * If a thread is found, it switches context to that thread. If no threads are ready
 * to run, it checks if there are any threads that are still BLOCKED or RUNNING.
 * If no threads are active, it exits the program.
 */
void schedule_next() {
    block();
    DEBUG_PRINT("[current_tid] Current thread ID: %d\n", current_tid);
    uthread_tcb_t* prev = &thread_table[current_tid];
    uthread_tcb_t* next = NULL;

    // what is the loop doing?
    // It iterates through the ready queue to find the next thread that is in the THREAD_READY state.
   
    while (!queue_is_empty(&ready_queue)) {
        next = (uthread_tcb_t*) queue_pop(&ready_queue);
        if (next->state == THREAD_READY)
            break;
        next = NULL;
    }

    DEBUG_PRINT("[scheduler] Switching from thread %d to thread %d\n", current_tid, next ? next->tid : -1);
    unblock();

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

    // DEBUG_PRINT("Switching from thread %d to thread %d\n", prev->tid, next->tid);
    // DEBUG_PRINT("address of prev->context: %p\n", (void*)&prev->context);
    // DEBUG_PRINT("address of next->context: %p\n", (void*)&next->context);


    swapcontext(&prev->context, &next->context);
}