#include "queue.h"
#include <stdlib.h>

void queue_init(queue_t* queue) {
    queue->head = queue->tail = NULL;
    queue->size = 0;
}

bool queue_is_empty(queue_t* queue) {
    return queue->size == 0;
}

int queue_size(queue_t *queue) {
    return queue->size;
}

bool queue_push(queue_t* queue, void* data) {
    queue_node_t* node = malloc(sizeof(queue_node_t));
    if (!node) return false;
    node->data = data;
    node->next = NULL;

    if (queue->tail)
        queue->tail->next = node;
    else
        queue->head = node;

    queue->tail = node;
    queue->size++;
    
    return true;
}

void* queue_pop(queue_t* queue) {
    if (queue_is_empty(queue)) return NULL;

    queue_node_t* node = queue->head;
    void* data = node->data;
    queue->head = node->next;
    if (!queue->head)
        queue->tail = NULL;

    free(node);
    queue->size--;

    return data;
}
