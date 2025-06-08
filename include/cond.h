#ifndef COND_H
#define COND_H

#include "queue.h"
#include "mutex.h"

typedef struct uthread_cond {
    queue_t waiters;
} uthread_cond_t;

void uthread_cond_init(uthread_cond_t *cond);
void uthread_cond_wait(uthread_cond_t *cond, uthread_mutex_t *mutex);
void uthread_cond_signal(uthread_cond_t *cond);
void uthread_cond_broadcast(uthread_cond_t *cond);

#endif
