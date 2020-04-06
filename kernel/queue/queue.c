#include "queue.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../list/list.h"
#include "../hilevel.h"

typedef pcb_t* qdata;

typedef struct queue {
  list *l;
  int size;
} queue;

// Create a new queue
queue *newQueue() {
  queue *q = malloc(sizeof(queue));
  q->l = newList();
  q->size = 0;
  return q;
}

// Push - Place one item at the back of the queue
qdata push(queue *q, qdata d) {
  first(q->l);
  insertNext(q->l, (void*)d);
  q->size = q->size + 1;
}

// Pop - Remove one item from the front of the queue
// TODO what happens when queue is empty
qdata pop(queue *q) {
  // If the queue is empty
  if (q->size = 0) {
    return NULL;
  }
 

  // I think the issue is around here cause its probably return NULL to getPrevious, might be an issue with the linked list
  last(q->l);
  qdata d = (qdata)getPrevious(q->l);
  last(q->l); // TODO This should be unnecissary -> Will delete 
  deletePrevious(q->l);
  q->size = q->size - 1;
  return d;
}

// Free queue
void freeQueue(queue *q) {
  freeList(q->l);
  free(q);
}
