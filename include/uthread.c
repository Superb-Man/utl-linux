#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "uthread.h"
#include "scheduler.h"


#define JB_SP 6
#define JB_PC 7


uthread_tcb_t thread_table[MAX_THREADS];  
uthread_t current_tid = -1;         
int thread_start = 0;        
int time_slice_ms = 10;
struct itimerval timer;
sigset_t signal_set;

void init();
void uthread_deinit();


void print_thread(uthread_tcb_t tcb) {
    DEBUG_PRINT("Thread ID: %d\n", tcb.tid);
    DEBUG_PRINT("State: %d\n", tcb.state);
    DEBUG_PRINT("Stack: %p\n", tcb.stack);  
    DEBUG_PRINT("Return Value: %p\n", tcb.retval);
    DEBUG_PRINT("Start Function: %p\n", tcb.start_func);
    DEBUG_PRINT("Argument: %p\n", tcb.arg);
    DEBUG_PRINT("Waiting Thread: %p\n", tcb.waiting_thread);

}

long long now_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

int get_tid(void) {
    // This function should return the current thread ID
    // Assuming current_tid is a global variable that holds the ID of the currently running thread
    return current_tid;
}


void thread_wrapper() {
    // This function is called when a thread is created
    // This function is called when a thread is created. It executes the thread's start function.

    DEBUG_PRINT("Thread started: %d\n", current_tid);
    uthread_tcb_t* tcb = &thread_table[current_tid];
    tcb->start_func(tcb->arg);
    uthread_exit(NULL); // Exit the thread when done

}


void 
timer_handler(int signum) {
    // This function is called when the timer expires
    // It should yield the current thread and schedule the next one
    // wake up handling
    long long now = now_ms();
    for (int i = 1; i < MAX_THREADS; ++i) {
        if (thread_table[i].state == THREAD_BLOCKED && thread_table[i].wakeup_time > 0
            && thread_table[i].wakeup_time <= now) {
            thread_table[i].state = THREAD_READY;
            thread_table[i].wakeup_time = 0;
            enqueue_thread(&thread_table[i]);
        }
    }
    printf("Timer expired for thread %d\n", current_tid);
    if (current_tid >= 0 && thread_table[current_tid].state == THREAD_RUNNING) {
        thread_table[current_tid].state = THREAD_READY;
        enqueue_thread(&thread_table[current_tid]);
        schedule_next();
    }
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
int 
uthread_create(void (*start_routine)(void* ), void* arg) {
    block(); 
    // uthread_run();
    for (int i = 1; i < MAX_THREADS; ++i) {

        DEBUG_PRINT("Thread %d stack: %p\n", i, thread_table[i].stack);

        if (thread_table[i].state == THREAD_UNUSED) { // if the thread is unused
            if (posix_memalign((void **)&thread_table[i].stack, 16, STACK_SIZE) != 0) {
                ERROR_PRINT("[uthread_create] Failed to allocate aligned stack for thread %d\n", i);
                unblock();
                ERROR_PRINT("[uthread_create] posix_memalign failed");
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
            // int active = 0;
            // for (int j = 1; j < MAX_THREADS; ++j) {
            //     if (thread_table[j].state == THREAD_READY || thread_table[j].state == THREAD_RUNNING)
            //         active++;
            // }
            enqueue_thread(&thread_table[i]);
            
            DEBUG_PRINT("[uthread_create] Created thread %d with stack at %p\n", i, thread_table[i].stack);
            // if (!thread_start) {
            //     DEBUG_PRINT("[uthread_create] Starting thread scheduler\n");
            //     uthread_deinit();
            // }

            DEBUG_PRINT("Queue size after creating thread %d: %d\n", i, queue_size(&ready_queue));
            if ((queue_size(&ready_queue) > 1)) {
                DEBUG_PRINT("[uthread_create] Scheduling next thread\n");
                // schedule_next();
                init();
            }
            unblock();

            return i;

        }
    }

    ERROR_PRINT("[uthread_create] No available thread slots\n");
    unblock();
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
void 
uthread_exit(void* retval) {
    block();
    uthread_tcb_t* tcb = &thread_table[current_tid];
    tcb->retval = retval;
    tcb->state = THREAD_ZOMBIE;

    if (tcb->waiting_thread) {
        tcb->waiting_thread->state = THREAD_READY;
        enqueue_thread(tcb->waiting_thread);
    }

    unblock();
    uthread_yield();
}


/**
 * @brief Yield the current thread.
 * 
 * This function allows the current thread to yield control to the scheduler.
 * It changes the state of the current thread to READY and enqueues it for scheduling.
 * User has to call this function to allow other threads to run.
 */
void 
uthread_yield() {
    block();
    uthread_tcb_t* current = &thread_table[current_tid];

    if (current->state == THREAD_RUNNING && current_tid != 0) {
        current->state = THREAD_READY;
        enqueue_thread(current);
    }

    DEBUG_PRINT("[uthread_yield] Yielding thread %d\n", current_tid);
    if (queue_size(&ready_queue) == 1) {
        DEBUG_PRINT("[uthread_yield] No other threads to schedule, continuing execution of thread %d\n", current_tid);
        uthread_deinit();
    }
    unblock();
    schedule_next(); 
}

void
uthread_sleep(int ms) {
    block();
    uthread_tcb_t* current = &thread_table[current_tid];
    current->wakeup_time = now_ms() + ms;
    current->state = THREAD_BLOCKED;
    unblock();
    schedule_next();
}

/**
 * @brief Waits for a thread to terminate and retrieves its return value.
 *
 * @param tid The ID of the thread to wait for.
 * @return The return value of the terminated thread, or NULL if the thread is not joinable.
 */
void* 
uthread_join(uthread_t tid) {
    if (tid < 0 || tid >= MAX_THREADS || thread_table[tid].state == THREAD_UNUSED ) {
        ERROR_PRINT("[uthread_join] Invalid thread ID: %d\n", tid);
        return NULL;
    }

    if (tid == current_tid) {
        ERROR_PRINT("[uthread_join] Cannot join the current thread: %d\n", tid);
        return NULL;
    }

    uthread_tcb_t* target = &thread_table[tid];

    if (target->state != THREAD_ZOMBIE) {
        block();
        uthread_tcb_t* current = &thread_table[current_tid];
        current->state = THREAD_BLOCKED;
        target->waiting_thread = current;

        DEBUG_PRINT("[uthread_join] Blocking thread %d, waiting for thread %d to finish\n", current_tid, tid);
        unblock();
        uthread_yield(); // Yield to allow the target thread to run
    }

    free(target->stack);
    target->state = THREAD_UNUSED; // Mark the thread as unused
    DEBUG_PRINT("[uthread_join] Thread %d has finished, return value: %p\n", tid, target->retval);

   return target->retval;
}

/**
 * @brief Initializes the signal handler and timer for thread scheduling.
 * 
 * This function sets up a signal handler for SIGALRM, which is used to trigger
 * the timer for thread scheduling. It also initializes the timer with the specified time slice.
 */
void 
init() {
    printf("[uthread_init] Initializing thread scheduler with time slice: %d ms\n", time_slice_ms);
    struct sigaction sa; 
    sa.sa_handler = timer_handler; // Set the signal handler for SIGALRM
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGALRM, &sa, NULL) == -1) { // Set up the signal handler
        ERROR_PRINT("[uthread_init] Failed to set up signal handler\n");
        exit(EXIT_FAILURE);
    }

    timer.it_value.tv_sec = time_slice_ms / 1000;
    timer.it_value.tv_usec = (time_slice_ms % 1000) * 1000;
    timer.it_interval = timer.it_value;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        ERROR_PRINT("[uthread_init] Failed to set up timer\n");
        exit(EXIT_FAILURE);
    }
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGALRM);
}

void uthread_deinit() {
    // Disable the timer
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
}

void 
uthread_run(void) {
    if (thread_start) {
        ERROR_PRINT("[uthread_run] Thread scheduler already started\n");
        return;
    }
    thread_start = 1;
    thread_table[0].tid = 0;
    thread_table[0].state = THREAD_RUNNING;
    thread_table[0].stack = NULL;
    thread_table[0].start_func = NULL;
    getcontext(&thread_table[0].context);
    current_tid = 0;

    // init();
    // uthread_deinit();
    schedule_next();
}
// gcc test.c include/uthread.c include/queue.c -o test

