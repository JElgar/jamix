#include "queue.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../list/list.h"

typedef pid_t* qdata;

typedef struct queue {
  list *l;
} queue;

// Create a new queue
queue *newQueue(qdata d) {
  queue *q = malloc(sizeof(queue));
  q->l = newList();
  return q;
}

// Push - Place one item at the back of the queue
qdata push(queue *q, qdata d) {
  first(q->l);
  insertPrevious(q->l, (void*)d);
}

// Pop - Remove one item from the front of the queue
qdata pop(queue *q) {
  last(q->l);
  qdata d = (qdata)getPrevious(q->l);
  deletePrevious(q->l);
  return d;
}

// Free queue
void freeQueue(queue *q) {
  freeList(q->l);
  free(q);
}
