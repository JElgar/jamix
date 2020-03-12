#include <stdbool.h>

// Type for elements in list
typedef int data;

typedef struct list list;

// Free up the list and all the data in it.
void freeList(list *l);

// Set the current position before the first item or after the last item
void first(list *l);
void last(list *l);

// Check whether the current position is at the end or start
bool isLast(list *l);
bool isFirst(list *l);

// Move the current position one place forwards or backwards, and return true.
// If next is called when at the end of the list, or previous when at the start,
// the functions do nothing and return 1
bool next(list *l);
bool previous(list *l);

// Insert an item before the current position during a traversal.  The traversal
// of the remaining items is not affected.
void insertNext(list *l, data x);
void insertPrevious(list *l, data x);

// Get the current item. If getF is called when at the end, or getB is called
// when at the start, the default item is returned.
data getNext(list *l);
data getPrevious(list *l);

// Set node value
bool setNext(list *l, data x);
bool setPrevious(list *l, data x);

// Delete node, 0 -> Success, 1 -> Failure
bool deleteNext(list *l);
bool deletePrevious(list *l);

// Empty list node for error handling (basically void)
list *createList(data d);
