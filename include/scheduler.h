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
        DEBUG_PRINT("[enqueue_thread] Enqueued thread %d at the end of queue\n", tcb->tid);

    }
}

/**
 * @brief Schedule the next thread to run
 * 
 * @details This function enters a scheduling loop:
 *   1. Wakes up any sleeping threads whose wakeup time has passed.
 *   2. Pops the next ready thread from the ready queue.
 *   3. If a thread is found, switches context to that thread via swapcontext.
 *   4. If no threads are ready but some are blocked (sleeping/joining), calculates
 *      the nearest wakeup time, disables the periodic timer, sleeps precisely
 *      for that duration, re-enables the timer, and loops back to step 1.
 *   5. If no threads are active at all, exits the program.
 */
void schedule_next() {
    block();
    DEBUG_PRINT("[Schedule next] [current_tid] Current thread ID: %d\n", current_tid);
    uthread_tcb_t* prev = &thread_table[current_tid];
    uthread_tcb_t* next = NULL;

    // Wake up any sleeping threads whose wakeup time has passed
    long long now = now_ms();
    for (int i = 1; i < MAX_THREADS; i++) {
        if (thread_table[i].state == THREAD_BLOCKED &&
            thread_table[i].wakeup_time > 0 &&
            thread_table[i].wakeup_time <= now) {
            thread_table[i].state = THREAD_READY;
            thread_table[i].wakeup_time = 0;
            enqueue_thread(&thread_table[i]);
            DEBUG_PRINT("[Schedule next] Waking up sleeping thread %d\n", i);
        }
    }

    // Pop the next ready thread from the queue
    while (!queue_is_empty(&ready_queue)) {
        next = (uthread_tcb_t*) queue_pop(&ready_queue);
        if (next->state == THREAD_READY)
            break;
        next = NULL;
    }

    if (next) {
        current_tid = next->tid;
        next->state = THREAD_RUNNING;
        DEBUG_PRINT("[Schedule next] Switching from thread %d to thread %d\n", prev->tid, next->tid);
        // DEBUG_PRINT("address of prev->context: %p\n", (void*)&prev->context);
        // DEBUG_PRINT("address of next->context: %p\n", (void*)&next->context);
        unblock();
        swapcontext(&prev->context, &next->context);

        return;
    }

    // No ready threads. Check if any blocked/running threads still exist.
    int any_active = 0;
    for (int i = 1; i < MAX_THREADS; i++) {
        if (thread_table[i].state == THREAD_BLOCKED || thread_table[i].state == THREAD_RUNNING) {
            any_active = 1;
            break;
        }
    }

    if (!any_active) {
        unblock();
        ERROR_PRINT("[Schedule next] All threads have finished.\n");
        exit(0);
    }

    long long now2 = now_ms();
    long long nearest_wakeup = 0;
    for (int i = 1; i < MAX_THREADS; i++) {
        if (thread_table[i].state == THREAD_BLOCKED && thread_table[i].wakeup_time > 0) {
            long long remaining = thread_table[i].wakeup_time - now2;
            if (remaining > 0 && (nearest_wakeup == 0 || remaining < nearest_wakeup))
                nearest_wakeup = remaining;
        }
    }

    uthread_deinit();
    unblock();
    if (nearest_wakeup > 0)
        usleep((useconds_t)nearest_wakeup * 1000);
    else
        usleep(10000);  // fallback (shouldn't normally reach here)
    block();
    init();  // re-enable timer
    schedule_next();
}