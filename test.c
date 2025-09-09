#include <stdio.h>
#include "include/uthread.h"

int cnt = 0;

// A dummy thread function
void dummy_thread(void* arg) {
    int id = *(int*)arg;
    // sleep(3);

    // simulate doing work
    // uthread_sleep(3);
    for (int i = 0; i < 20000000; i++) {
        // cnt++;
        // yield to let other threads (including main) run
        // uthread_yield();
    }

    printf("Thread %d finished, final cnt = %d\n", id, cnt);
}

void main_thread_func(void* arg) {
    // start user thread scheduler

    int ids[3] = {1, 2, 3};

    // create three dummy threads
    int t1 = uthread_create(dummy_thread, &ids[0]);
    // int t2 = uthread_create(dummy_thread, &ids[1]);
    // int t3 = uthread_create(dummy_thread, &ids[2]);
    // sleep(2);
    // uthread_run();


    // main thread also does some work
    for (int i = 0; i < 2000000; i++) {
        cnt++;
        printf("Main thread: iteration %d, cnt = %d\n", i, cnt);

        // uthread_yield();
    }
    // uthread_join(t1);
    // uthread_join(t2);
    // uthread_join(t3);

    printf("All threads finished. Final cnt = %d\n", cnt);
}

int main() {
    // create the main thread
    uthread_create(main_thread_func, NULL);
    // start the thread scheduler
    // uthread_run();
}
