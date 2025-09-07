#include <stdio.h>
#include "include/uthread.h"

int cnt = 0;

void dummy_thread(void* arg) {
    sleep(3);
    uthread_yield();
    int id = *(int* )arg;
    printf("arg: %d\n", id);
    printf("Value of cnt: %d\n", cnt); // should not be 0
}

int main() {
    // get the stack address of this main
    void* main_stack = __builtin_frame_address(0);
    // initialize the threading system with the main stack and main function
    uthread_run(main_stack, (void (*)(void))main);
    int ids[3] = {1, 2, 3};
    int t1 = uthread_create(dummy_thread, &ids[0]);
    uthread_yield();
    // int t2 = uthread_create(dummy_thread, &ids[1]);
    // int t3 = uthread_create(dummy_thread, &ids[2]);
    // uthread_run();
    // uthread_run(main_stack);
    for (int i = 0; i < 10; i++) {
        cnt++;
        printf("Main thread: iteration %d, cnt = %d\n", i, cnt);
    }

    return 0;
}
