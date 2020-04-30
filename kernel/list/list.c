#include "list.h"

list *newList(data d) {
  list *l = malloc(sizeof(list));
  item *s = malloc(sizeof(item));


  l->sentinel = s;
  l->current = s;

  s->previous = s;
  s->next = s;
  s->value = (data)NULL;

  return l;
}

void first(list *l) {
  l->current = l->sentinel->next;
}

void last(list *l) {
  l->current = l->sentinel;
}

bool isLast(list *l) {
  return (l->current->value == NULL);
}

bool isFirst(list *l) {
  return (l->current->previous->value == NULL);
}

bool next(list *l) {
  if(isLast(l)) {
    return false;
  } else {
    l->current = l->current->next;
    return true;
  }
}

bool previous(list *l) {
  if(isFirst(l)) {
    return false;
  } else {
    l-> current = l->current->previous;
    return true;
  }
}

void insertNext(list *l, data x) {
  item *new = malloc(sizeof(item));
  new->value = x;

  item *current = l->current;
  item *previous = current->previous;

  new->next = current;
  new->previous = previous;

  previous->next = new;
  current->previous = new;
}

void insertPrevious(list *l, data x) {
  item *new = malloc(sizeof(item));
  new->value = x;

  item *current = l->current;
  item *previous = current->previous;

  new->next = current;
  new->previous = previous;

  previous->next = new;
  current->previous = new;

  l->current = new;
}

data getNext(list *l) {
  if(isLast(l)) return NULL;
  else return l->current->value;
}
data getPrevious(list *l) {
  if(isFirst(l)) return NULL;
  else return l->current->previous->value;
}

bool setNext(list *l, data x) {
  if(isLast(l)) return false;
  else {
    l->current->value = x;
    return true;
  }
}

bool setPrevious(list *l, data x) {
  if(isFirst(l)) return false;
  else {
    l->current->previous->value = x;
    return true;
  }
}

bool deleteNext(list *l) {
  if (isLast(l)) return false;

  item *current = l->current;
  item *next = current->next;
  item *previous = current->previous;

  next->previous = previous;
  previous->next = next;

  //free(current->value);
  free(current);

  l->current = next;
  return true;
}

bool deletePrevious(list *l) {
  if (isFirst(l)) return false;

  item *current = l->current->previous;
  item *next = current->next;
  item *previous = current->previous;

  next->previous = previous;
  previous->next = next;

  //free(current->value);
  free(current);

  return true;
}

void freeList(list *l) {
  item *i = l->sentinel->next;
  while (i->value != (data)NULL) {
    item *next = i->next; 
    //free(i->value);
    free(i);
    i = next;
  }
  free(l->sentinel);
  free(l);
}
