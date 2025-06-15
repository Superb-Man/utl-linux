#ifndef MUTEX_H
#define MUTEX_H
#include "queue.h"

typedef struct uthread_mutex {
    int locked;
    queue_t waiters;
} uthread_mutex_t;

void uthread_mutex_init(uthread_mutex_t* mutex);
void uthread_mutex_lock(uthread_mutex_t* mutex);
void uthread_mutex_unlock(uthread_mutex_t* mutex);

#endif
