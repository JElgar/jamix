#include "./priorityQueue.h"


// Create a new queue
priorityQueue *newPriorityQueue() {
  priorityQueue *pq = malloc(sizeof( priorityQueue ));
  pq->l = newList();
  pq->size = 0;
  return pq;
}

// Push - Place one item at the back of the queue
void pqPush(priorityQueue *pq, pqdata d, int priority) {
  first(pq->l);
  pqitem *item = malloc(sizeof(pqitem));
  item->data = d;
  item->priority = priority;

  // Bigger number = less important
  // Go until the next item has lower priority that this 
  if (pq->size > 0 ) {
    int currentp = ((struct pqitem*) getNext(pq->l))->priority;
    while ( priority < currentp ) {
      next(pq->l);
      currentp = ((struct pqitem*) getNext(pq->l))->priority;
    }
  }
  insertNext(pq->l, (void*)item);
  pq->size = pq->size + 1;
}

// Pop - Remove one item from the front of the queue
pcb_t *pqPop(priorityQueue *pq) {
  // If the queue is empty
  if (pq->size == 0) {
    return NULL;
  }
 
  last(pq->l);
  pqitem *i = (struct pqitem*) getPrevious(pq->l);
  deletePrevious(pq->l);
  pq->size = pq->size - 1;
  pqitem *p = (struct pqitem*)i->data;
  //free(i);
  return p->data;
}

// Look at the next item in the queue, if its 
int pqPeekPriority(priorityQueue *pq) {
  // If the queue is empty
  if (pq->size == 0) {
    return NULL;
  }
 
  last(pq->l);
  return ((struct pqitem*) getPrevious(pq->l))->priority;
}

void deleteItem (priorityQueue *pq, uint32_t pid) {
  first(pq->l);
  pqitem* del = ((struct pqitem*) getNext(pq->l));
  while(del->data->pid != pid) {
    next(pq->l);
    del = ((struct pqitem*) getNext(pq->l));
  }
  deleteNext(pq->l);
  pq->size--;
}

// Free queue
void freePriorityQueue(priorityQueue *pq) {
  freeList(pq->l);
  // TODO check no memory leak here
  free(pq);
}
