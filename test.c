#include <stdio.h>
#include "include/uthread.h"

long cnt = 0;
int finished = 0;
// A dummy thread function
void dummy_thread(void* arg) {
    int id = *(int*)arg;
    // sleep(3);

    // simulate doing work
    // uthread_sleep(3);
    for (int i = 0; i < 200000000; i++) {
        cnt++;
        // yield to let other threads (including main) run
        // uthread_yield();
        // if (finished) {
        //     printf("Thread %d exiting early, cnt = %ld\n", id, cnt);
        //     break;
        // }
    }

    printf("Thread %d finished, final cnt = %ld\n", id, cnt);
    finished++;
}

void dummy_thread2(void* arg) {
    int id = *(int*)arg;
    // sleep(3);

    // simulate doing work
    // uthread_sleep(3);
    for (int i = 0; i < 10000000; i++) {
        cnt++;
        // yield to let other threads (including main) run
        // uthread_yield();
        // if (finished) {
        //     printf("Thread %d exiting early, cnt = %ld\n", id, cnt);
        //     break;
        // }
    }

    printf("Thread %d finished, final cnt = %ld\n", id, cnt);
    finished++;
}

void main_thread_func(void* arg) {


    // start user thread scheduler

    int ids[2] = {2, 3};

    // create three dummy threads
    int t1 = uthread_create(dummy_thread, &ids[0]);
    int t2 = uthread_create(dummy_thread2, &ids[1]);
    // int t3 = uthread_create(dummy_thread, &ids[2]);
    // sleep(2);
    // uthread_run();
    for (int i = 0; i <= 10000000; i++) {
        // printf("My name is Lara\n");
    }


    // main thread also does some work
    for (int i = 0; i < 200000000; i++) {
        cnt++;
        // printf("Main thread: iteration %d, cnt = %d\n", i, cnt);

        // uthread_yield();
    }
    uthread_join(t1);
    uthread_join(t2);
    // uthread_join(t3);

    printf("Final cnt = %ld\n", cnt);
    finished++;
    uthread_sleep(5000);
    if (finished == 3) {
        printf("Main thread exiting, cnt = %ld\n", cnt);
    }
}

int main() {
    // create the main thread
    uthread_create(main_thread_func, NULL);
    uthread_run();
    // start the thread scheduler
    // uthread_run();
}
