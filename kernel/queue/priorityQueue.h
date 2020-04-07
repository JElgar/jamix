#include "../hilevel.h"

typedef pcb_t* pqdata;

typedef struct pqueue pqueue;

typedef struct pqitem {
  pqdata data;
  int priority;
} pqitem;

// Create a new queue
pqueue *newPriorityQueue();

// Push - Place one item at the back of the queue
void pqPush(pqueue *pq, pqdata d, int p);

// Pop - Remove one item from the front of the queue
pqdata pqPop(pqueue *q);

// Free queue
void freePriorityQueue(queue *q);
