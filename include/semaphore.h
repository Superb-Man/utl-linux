#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "queue.h"

typedef struct {
    int value;
    queue_t waiters;
} uthread_sem_t;

void uthread_sem_init(uthread_sem_t *sem, int value);
void uthread_sem_post(uthread_sem_t *sem);
void uthread_sem_wait(uthread_sem_t *sem);

#endif 
