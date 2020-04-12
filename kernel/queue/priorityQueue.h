#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../list/list.h"
#include "../hilevel.h"


typedef pcb_t* pqdata;

typedef struct priorityQueue {
  list *l;
  int size;
} priorityQueue;

typedef struct pqitem {
  pqdata data;
  int priority;
} pqitem;

// Create a new queue
priorityQueue *newPriorityQueue();

// Push - Place one item at the back of the queue
void pqPush(priorityQueue *pq, pqdata d, int p);

// Pop - Remove one item from the front of the queue
pqitem *pqPop(priorityQueue *pq);

// Peek - return (without removing) first item from in queue
pqitem *pqPeek(priorityQueue *pq);

// Delete the process with the given pid
void deleteItem(priorityQueue *pq, uint32_t pid);

// Free queue
void freePriorityQueue(priorityQueue *pq);
