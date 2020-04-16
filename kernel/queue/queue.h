#include "../hilevel.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../list/list.h"
#include "../hilevel.h"

typedef void* qdata;

typedef struct queue queue;

// Create a new queue
queue *newQueue();

// Push - Place one item at the back of the queue
void push(queue *q, qdata d);

// Pop - Remove one item from the front of the queue
qdata pop(queue *q);

// Free queue
void freeQueue(queue *q);
