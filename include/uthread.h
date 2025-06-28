#ifndef UTHREAD_H
#define UTHREAD_H

#include <setjmp.h>
#include "mutex.h"
#include "cond.h"
#include <ucontext.h>
#include "debug.h"

#define STACK_SIZE 4096
#define MAX_THREADS 64

typedef int uthread_t;

typedef enum {
    THREAD_UNUSED,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_ZOMBIE
} thread_state_t;

/**
 * uthread_tcb_t - Thread Control Block structure
 * Represents a thread's state, stack, and other attributes.
 * * @tid: Thread ID
 * * @stack: Pointer to the thread's stack
 * * @retval: Return value after thread exits
 * * @state: Current state of the thread (READY, RUNNING, BLOCKED, ZOMBIE, UNUSED)
 * * @start_func: Function to execute when the thread starts
 * * @arg: Argument to pass to the start function
 * * @waiting_thread: Pointer to the thread that is waiting for this thread to finish
 * * @context: Context for the thread, used for switching between threads
 */

typedef struct uthread {
    uthread_t tid;
    void* stack;
    void* retval;
    thread_state_t state;
    void (*start_func)(void*);
    void* arg;
    struct uthread* waiting_thread;
    ucontext_t context;
} uthread_tcb_t;

void print_thread(uthread_tcb_t tcb); // for debugging purposes
int uthread_create(void (*start_routine)(void*), void* arg);
int uthread_join(uthread_t tid, void** retval);
void uthread_exit(void* retval); // exit the current thread
uthread_t get_tid(void); // get the current thread ID
void uthread_yield(void); // yield the CPU to another thread
void uthread_run(void); // start the thread scheduler

extern uthread_tcb_t thread_table[MAX_THREADS];
extern uthread_t current_tid;

#endif
