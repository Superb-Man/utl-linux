#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>

#include "uthread.h"
#include "scheduler.h"


#define JB_SP 6
#define JB_PC 7


uthread_tcb_t thread_table[MAX_THREADS];  
uthread_t current_tid = 0;                



void print_thread(uthread_tcb_t tcb) {
    DEBUG_PRINT("Thread ID: %d\n", tcb.tid);
    DEBUG_PRINT("State: %d\n", tcb.state);
    DEBUG_PRINT("Stack: %p\n", tcb.stack);  
    DEBUG_PRINT("Return Value: %p\n", tcb.retval);
    DEBUG_PRINT("Start Function: %p\n", tcb.start_func);
    DEBUG_PRINT("Argument: %p\n", tcb.arg);
    DEBUG_PRINT("Waiting Thread: %p\n", tcb.waiting_thread);
    DEBUG_PRINT("Context: %p\n", &tcb.context);
}

int get_tid(void) {
    // This function should return the current thread ID
    // Assuming current_tid is a global variable that holds the ID of the currently running thread
    return current_tid;
}

void thread_wrapper(void) {
    DEBUG_PRINT("Thread started: %d\n", current_tid);

    // This function is the entry point for the thread
    uthread_tcb_t* tcb = &thread_table[current_tid];
    tcb->start_func(tcb->arg);
    uthread_exit(NULL); // Exit the thread when done
}

int uthread_create(void (*start_routine)(void* ), void* arg) {

    for (int i = 1; i < MAX_THREADS; ++i) {

        DEBUG_PRINT("Thread %d stack: %p\n", i, thread_table[i].stack);

        if (thread_table[i].state == THREAD_UNUSED) { // if the thread is unused
            if (posix_memalign((void **)&thread_table[i].stack, 16, STACK_SIZE) != 0) {
                ERROR_PRINT("Failed to allocate aligned stack for thread %d\n", i);
                return -1;
            }
            

            thread_table[i].tid = i;
            thread_table[i].state = THREAD_READY;
            thread_table[i].start_func = start_routine;
            thread_table[i].arg = arg;
            thread_table[i].waiting_thread = NULL;


            ucontext_t* ctx = &thread_table[i].context;
            getcontext(ctx);

            ctx->uc_stack.ss_sp = thread_table[i].stack;
            ctx->uc_stack.ss_size = STACK_SIZE;
            ctx->uc_link = NULL;
            makecontext(ctx, (void (*)(void))thread_wrapper, 0);

            enqueue_thread(&thread_table[i]);
            DEBUG_PRINT("Created thread %d with stack at %p\n", i, thread_table[i].stack);
            return i;

        }
    }

    ERROR_PRINT("No available thread slots\n");
    return -1;
}

void uthread_exit(void* retval) {

    uthread_tcb_t* tcb = &thread_table[current_tid];
    tcb->retval = retval;
    tcb->state = THREAD_ZOMBIE;

    if (tcb->waiting_thread) {
        tcb->waiting_thread->state = THREAD_READY;
        enqueue_thread(tcb->waiting_thread);
    }

    uthread_yield();
}

void uthread_yield(void) {
    uthread_tcb_t* current = &thread_table[current_tid];

    if (current->state == THREAD_RUNNING && current_tid != 0) {
        current->state = THREAD_READY;
        enqueue_thread(current);
    }

    DEBUG_PRINT("[uthread_yield] Yielding thread %d\n", current_tid);
    schedule_next(); 
}


void uthread_run(void) {
    thread_table[0].tid = 0;
    thread_table[0].state = THREAD_RUNNING;
    thread_table[0].stack = NULL;
    getcontext(&thread_table[0].context);

    current_tid = 0;
    schedule_next();
}