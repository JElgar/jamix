#include <stdbool.h>

typedef int pid_t;
typedef pid_t* qdata;

typedef struct queue queue;

// Create a new queue
queue *newQueue(qdata d);

// Push - Place one item at the back of the queue
qdata push(queue *q, qdata d);

// Pop - Remove one item from the front of the queue
qdata pop(queue *q);

// Free queue
void freeQueue(queue *q);
