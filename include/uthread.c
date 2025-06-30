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
int thread_start = 0;        



void print_thread(uthread_tcb_t tcb) {
    DEBUG_PRINT("Thread ID: %d\n", tcb.tid);
    DEBUG_PRINT("State: %d\n", tcb.state);
    DEBUG_PRINT("Stack: %p\n", tcb.stack);  
    DEBUG_PRINT("Return Value: %p\n", tcb.retval);
    DEBUG_PRINT("Start Function: %p\n", tcb.start_func);
    DEBUG_PRINT("Argument: %p\n", tcb.arg);
    DEBUG_PRINT("Waiting Thread: %p\n", tcb.waiting_thread);

}

int get_tid(void) {
    // This function should return the current thread ID
    // Assuming current_tid is a global variable that holds the ID of the currently running thread
    return current_tid;
}

void uthread_init() {
    thread_table[0].tid = 0;
    thread_table[0].state = THREAD_RUNNING;
    thread_table[0].stack = NULL; 
    thread_table[0].waiting_thread = NULL;
    current_tid = 0;
}


/**
 * @brief Thread wrapper function.
 * 
 * This function is called when a thread is created. It executes the thread's start function.
 * 
 */
void thread_wrapper() {
    DEBUG_PRINT("Thread started: %d\n", current_tid);
    uthread_tcb_t* tcb = &thread_table[current_tid];
    tcb->start_func(tcb->arg);
    uthread_exit(NULL); // Exit the thread when done
}

/**
 * @brief Create a new thread.
 * 
 * This function allocates a new thread control block, initializes it, and sets up the context for the new thread.
 * It uses `posix_memalign` to allocate a stack for the thread, sets the thread's state to READY,
 * JMP
 * and prepares the context for the thread to run its start function.
 * @param start_routine The function to be executed by the new thread.
 * @param arg The argument to be passed to the start function.
 * @return The thread ID of the newly created thread, or -1 if no slots are available.
 */
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


/**
 * @brief Exit the current thread.
 * 
 * This function changes the state of the current thread to ZOMBIE,
 * sets the return value, and wakes up any thread that was waiting for this thread to finish.
 * When a thread calls this function, it will not return to the caller.
 * It will switch to the next thread in the ready queue.
 *
 * @param retval The return value of the thread.
 * 
 * Stores the return value in the thread's TCB.
 * If the thread was joined by another thread, this value will be returned to that thread.
 */
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


/**
 * @brief Yield the current thread.
 * 
 * This function allows the current thread to yield control to the scheduler.
 * It changes the state of the current thread to READY and enqueues it for scheduling.
 * User has to call this function to allow other threads to run.
 */
void uthread_yield() {
    uthread_tcb_t* current = &thread_table[current_tid];

    if (current->state == THREAD_RUNNING && current_tid != 0) {
        current->state = THREAD_READY;
        enqueue_thread(current);
    }

    DEBUG_PRINT("[uthread_yield] Yielding thread %d\n", current_tid);
    schedule_next(); 
}

/**
 * @brief Waits for a thread to terminate and retrieves its return value.
 *
 * @param tid The ID of the thread to wait for.
 * @return The return value of the terminated thread, or NULL if the thread is not joinable.
 */
void* uthread_join(uthread_t tid) {
    if (tid < 0 || tid >= MAX_THREADS || thread_table[tid].state == THREAD_UNUSED ) {
        ERROR_PRINT("Invalid thread ID: %d\n", tid);
        return NULL;
    }

    if (tid == current_tid) {
        ERROR_PRINT("Cannot join the current thread: %d\n", tid);
        return NULL;
    }

    uthread_tcb_t* target = &thread_table[tid];

    if (target->state != THREAD_ZOMBIE) {
        uthread_tcb_t* current = &thread_table[current_tid];
        current->state = THREAD_BLOCKED;
        target->waiting_thread = current;

        DEBUG_PRINT("[uthread_join] Blocking thread %d, waiting for thread %d to finish\n", current_tid, tid);
        uthread_yield(); // Yield to allow the target thread to run
    }

    free(target->stack);
    target->state = THREAD_UNUSED; // Mark the thread as unused
    DEBUG_PRINT("[uthread_join] Thread %d has finished, return value: %p\n", tid, target->retval);

   return target->retval;
}