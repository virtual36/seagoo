#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	void ** data;
	int front;
	int rear;
	int size;
	int count;
} circular_queue;

int circular_queue_init(circular_queue * q, int size);
int circular_queue_destroy(circular_queue * q);
int circular_queue_is_empty(circular_queue * q);
int circular_queue_is_full(circular_queue * q);
int circular_queue_enqueue(circular_queue * q, void * item);
int circular_queue_dequeue(circular_queue * q, void ** item);

#endif  // CIRCULAR_QUEUE_H
