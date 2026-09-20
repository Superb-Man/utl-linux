#include <stdio.h>
#include <stdlib.h>
#include "include/uthread.h"
#include "include/mutex.h"
#include "include/semaphore.h"

#define BUFFER_SIZE        5
#define NUM_PRODUCERS      2
#define NUM_CONSUMERS      3
#define ITEMS_PER_PRODUCER 10000
#define TOTAL_ITEMS        (NUM_PRODUCERS * ITEMS_PER_PRODUCER)
#define POISON_PILL        (-1) // sentinel telling a consumer to stop

static int buffer[BUFFER_SIZE];
static int in = 0, out = 0;

static uthread_mutex_t buffer_lock;
static uthread_sem_t empty_slots;
static uthread_sem_t full_slots;

static uthread_mutex_t stat_lock;
static long produced_sum = 0;
static long consumed_sum = 0;
static int produced_count = 0;
static int consumed_count = 0;

static void buffer_put(int value) {
    uthread_sem_wait(&empty_slots);
    uthread_mutex_lock(&buffer_lock);
    buffer[in] = value;
    in = (in + 1) % BUFFER_SIZE;
    uthread_mutex_unlock(&buffer_lock);
    uthread_sem_post(&full_slots);
}

static int buffer_get(void) {
    uthread_sem_wait(&full_slots);
    uthread_mutex_lock(&buffer_lock);
    int value = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    uthread_mutex_unlock(&buffer_lock);
    uthread_sem_post(&empty_slots);
    return value;
}

void producer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = id * 100 + i;
        buffer_put(item);
        printf("[Producer %d] produced %d\n", id, item);

        uthread_mutex_lock(&stat_lock);
        produced_count++;
        produced_sum += item;
        uthread_mutex_unlock(&stat_lock);

        uthread_yield();
    }
}

void consumer(void* arg) {
    int id = *(int*)arg;
    while (1) {
        int item = buffer_get();
        if (item == POISON_PILL) {
            printf("[Consumer %d] received stop signal\n", id);
            break;
        }
        printf("[Consumer %d] consumed %d\n", id, item);

        uthread_mutex_lock(&stat_lock);
        consumed_count++;
        consumed_sum += item;
        uthread_mutex_unlock(&stat_lock);

        uthread_yield();
    }
}

void supervisor(void* arg) {
    (void)arg;
    uthread_mutex_init(&buffer_lock);
    uthread_mutex_init(&stat_lock);
    uthread_sem_init(&empty_slots, BUFFER_SIZE);
    uthread_sem_init(&full_slots, 0);

    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];
    int producer_tids[NUM_PRODUCERS];
    int consumer_tids[NUM_CONSUMERS];

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i;
        consumer_tids[i] = uthread_create(consumer, &consumer_ids[i]);
    }
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        producer_tids[i] = uthread_create(producer, &producer_ids[i]);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        uthread_join(producer_tids[i]);
    }

    // production is finished: wake every consumer with a poison pill
    // instead of a shared counter, so nobody can block forever
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        buffer_put(POISON_PILL);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        uthread_join(consumer_tids[i]);
    }

    int passed = produced_count == TOTAL_ITEMS &&
                 consumed_count == TOTAL_ITEMS &&
                 produced_sum == consumed_sum;

    printf("\n=== Producer-Consumer Test %s ===\n", passed ? "PASSED" : "FAILED");
    printf("Produced: %d items (sum=%ld)\n", produced_count, produced_sum);
    printf("Consumed: %d items (sum=%ld)\n", consumed_count, consumed_sum);
}

int main() {
    uthread_create(supervisor, NULL);
    uthread_run();
    return 0;
}       