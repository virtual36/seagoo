#include "circular_queue.h"

// Initialize the circular queue with a fixedlength
int circular_queue_init(circular_queue *q, int size) {
    q->data = (void **)malloc(size * sizeof(int));
    if (!q->data) {
        fprintf(stderr, "err: Circular queue malloc failed.\n");
        return -1;
    }
    q->front = 0;
    q->rear = 0;
    q->size = size;
    q->count = 0;
    return 0;
}

// Destroy the circular queue and free the allocated memory
int circular_queue_destroy(circular_queue *q) {
    free(q->data);
    q->data = NULL;
    return 0;
}

// Check if the queue is empty
int circular_queue_is_empty(circular_queue *q) {
    return (q->count == 0) ? 1 : 0;
}

// Check if the queue is full
int circular_queue_is_full(circular_queue *q) {
    return (q->count == q->size) ? 1 : 0;
}

// Enqueue an item into the circular queue
int circular_queue_enqueue(circular_queue *q, void * item) {
    if (circular_queue_is_full(q)) {
        fprintf(stderr, "err: Ciruclar Queue is full. Cannot enqueue %d.\n", item);
        return -1;
    }
    q->data[q->rear] = item;
    q->rear = (q->rear + 1) % q->size;
    q->count++;
    return 0;
}

// Dequeue an item from the circular queue
int circular_queue_dequeue(circular_queue *q, void ** item) {
    if (circular_queue_is_empty(q)) {
        fprintf(stderr, "err: Circular queue is empty. Cannot dequeue.\n");
        return -1;
    }
    *item = q->data[q->front];
    q->front = (q->front + 1) % q->size;
    q->count--;
    return 0;
}