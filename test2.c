#include <stdio.h>
#include "include/uthread.h"

void compute_prime_factors(int start, int end) {
    int cnt = 0;
    for (int num = start; num <= end; num++) {
        printf("Prime factors of %d: ", num);
        int n = num;
        for (int i = 2; i * i <= n; i++) {
            while (n % i == 0) {
                printf("%d ", i);
                n /= i;
            }
            cnt++;
        //     if (n == 1) {
        //         break; // No more factors left
        //     }
        }
        if (n > 1) {
            printf("%d ", n);
            cnt++;
        }
        printf("\n");
        uthread_yield();
        // sleep(5);
    }
    printf("Total prime factors in range %d-%d: %d\n", start, end, cnt);
}

void thread_prime_factors(void* arg) {
    int* range = (int*)arg;
    printf("Thread for range %d-%d started.\n", range[0], range[1]);
    compute_prime_factors(range[0], range[1]);
}

int main() {
    int ranges[6][2] = {
        {1, 10},
        {61, 70},
        {71, 80},
        {81, 90},
        {91, 100},
        {51, 60}
    };

    int thread_ids[6];
    for (int i = 0; i < 5; i++) {
        thread_ids[i] = uthread_create(thread_prime_factors, ranges[i]);
        if (thread_ids[i] < 0) {
            printf("Failed to create thread for range %d-%d\n", ranges[i][0], ranges[i][1]);
            return -1;
        }
    }
    thread_ids[5] = uthread_create(thread_prime_factors, ranges[5]);

    uthread_run();

    // for (int i = 0; i < 5; i++) {
    //     void* retval = uthread_join(thread_ids[i]);
    //     if (retval != NULL) {
    //         printf("Thread %d completed with return value: %p\n", thread_ids[i], retval);
    //     } 
    //     else {
    //         printf("Thread %d did not return a value.\n", thread_ids[i]);
    //     }
    // }
    return 0;
}