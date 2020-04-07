#include "queue.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../list/list.h"
#include "../hilevel.h"

typedef pcb_t* pqdata;

typedef struct pqueue {
  list *l;
  int size;
} pqueue;

typedef struct pqitem {
  pqdata data;
  int priority;
} pqitem;



// Create a new queue
pqueue *newPriorityQueue() {
  pqueue *pq = malloc(sizeof(pqueue));
  pq->l = newList();
  pq->size = 0;
  return pq;
}

// Push - Place one item at the back of the queue
void pqPush(pqueue *pq, pqdata d, int priority) {
  first(pq->l);
  pqitem *item = malloc(sizeof(pqitem));
  item->data = d;
  item->priority = priority;

  // Bigger number = less important
  // Go until the next item has lower priority that this 
  int currentp = ((pqitem*) getNext(pq->l))->priority;
  while (currentp < priority) {
    next(pq->l);
    int currentp = ((pqitem*) getNext(pq->l))->priority;
  }
  insertNext(pq->l, (void*)item);
  pq->size = pq->size + 1;
}

// Pop - Remove one item from the front of the queue
// TODO what happens when queue is empty
pqdata pqPop(pqueue *pq) {
  // If the queue is empty
  if (pq->size = 0) {
    return NULL;
  }
 
  // TODO check what im on about
  // I think the issue is around here cause its probably return NULL to getPrevious, might be an issue with the linked list
  last(pq->l);
  pqitem *i = (pqitem*) getPrevious(pq->l);
  last(pq->l); // TODO This should be unnecissary -> Will delete 
  deletePrevious(pq->l);
  pq->size = pq->size - 1;
  return i;
}

// Look at the next item in the queue, if its 
pqdata pqPeep(pqueue *pq) {
  // If the queue is empty
  if (pq->size = 0) {
    return NULL;
  }
 
  last(pq->l);
  pqitem *i = (pqitem*) getPrevious(pq->l);
  return i;
}

// Free queue
void freePriorityQueue(pqueue *pq) {
  freeList(pq->l);
  // TODO check no memory leak here
  free(pq);
}
