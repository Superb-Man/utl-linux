#include <stdio.h>
#include "include/uthread.h"

int cnt = 0;

void dummy_thread(void* arg) {
    int id = *(int* )arg;
    uthread_yield();
    printf("arg: %d\n", id);
    printf("Value of cnt: %d\n", cnt);
}

void main_thread_func(void* arg) {
    int ids[3] = {1, 2, 3};
    int t1 = uthread_create(dummy_thread, &ids[0]);
    uthread_yield();
    int t2 = uthread_create(dummy_thread, &ids[1]);
    int t3 = uthread_create(dummy_thread, &ids[2]);
    for (int i = 0; i < 10; i++) {
        cnt++;
        printf("Main thread: iteration %d, cnt = %d\n", i, cnt);
    }

    // join the threads
    uthread_join(t1);
    uthread_join(t2);
    uthread_join(t3);

    for (int i = 0; i < 10; i++) {
        cnt++;
        printf("Main thread: iteration %d, cnt = %d\n", i, cnt);
    }
}

int main() {
    // create the main thread
    uthread_create(main_thread_func, NULL);
    uthread_run();
    // uthread_join(main_thread);
}
