#include <stdio.h>
#include "include/uthread.h"

void dummy_thread(void* arg) {
    int id = *(int* )arg;
    printf("arg: %d\n", id);
    for (int i = 0; i < 3; ++i) {
        printf("Thread %d: iteration %d\n", id, i);
        uthread_yield();  // Cooperative yield to allow another thread to run
    }
}

int main() {
    int ids[3] = {1, 2, 3};
    int t1 = uthread_create(dummy_thread, &ids[0]);
    int t2 = uthread_create(dummy_thread, &ids[1]);
    int t3 = uthread_create(dummy_thread, &ids[2]);

    if (t1 >= 0 && t2 >= 0 && t3 >= 0) {
        printf("Successfully created threads: %d, %d, %d\n", t1, t2, t3);
        uthread_run(); // Start the thread scheduler
    } 
    else {
        printf("Failed to create threads.\n");
    }
    return 0;
}
