#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node_t;

typedef struct queue {
    queue_node_t *head;
    queue_node_t *tail;
    int size;
} queue_t;

void queue_init(queue_t *queue);
bool queue_is_empty(queue_t *queue);
int  queue_size(queue_t *queue);
bool queue_push(queue_t *queue, void *data);
void *queue_pop(queue_t *queue);

#endif
