#include <stdlib.h>
#include "list.h"
#include <stdbool.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef int data;

typedef struct item {
	struct item *previous;
	data value;
	struct item *next;
} item;

typedef struct list {
	item *sentinel;
	item *current;
    data def;
} list;

list *createList(data d) {
  list *l = malloc(sizeof(list));
  item *s = malloc(sizeof(item));


  l->sentinel = s;
  l->current = s;
  l->def = d; 

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
  return (l->current->value == (data)NULL);
}

bool isFirst(list *l) {
  return (l->current->previous->value == (data)NULL);
}

bool next(list *l) {
  if(isLast(l)) {
    return false;
  } else {
    l->current->next;
    return true;
  }
}

bool previous(list *l) {
  if(isFirst(l)) {
    return false;
  } else {
    l->current->previous;
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
  if(isLast(l)) return l->def;
  else return l->current->value;
}
data getPrevious(list *l) {
  if(isFirst(l)) return l->def;
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
  item *next = l->current->next;
  item *previous = l->current->previous;

  next->previous = previous;
  previous->next = next;

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

  free(current);

  return true;
}

void freeList(list *l) {
  item *i = l->sentinel->next;
  while (i->value != (data)NULL) {
    item *next = i->next; 
    free(i);
    i = next;
  }
  free(l->sentinel);
  free(l);
}


//// --- TESTING taken from COMS10008_2018 --- //
//
//// Testing for the lists module. Strings are used to describe lists. The strings
//// "|37", "3|7", "37|" represent a list of two items, with the current position
//// at the start, middle or end.
//#ifdef test_list
//
//// Convert a string description to a list.
//static list *toList(char *pic) {
//    int pos = strchr(pic, '|') - pic;
//    list *l = createList(-1);
//    for (int i = 0; i < strlen(pic); i++) {
//        if (pic[i] == '|') continue;
//        int n = pic[i] - '0';
//        insertNext(l, n);
//    }
//    first(l);
//    for (int i = 0; i < pos; i++) next(l);
//    return l;
//}
//
//// Convert a list to a string description.
//static void toString(list *l, char s[]) {
//    int pos = 0;
//    while (! isFirst(l)) { pos++; previous(l); }
//    int i = 0;
//    while (! isLast(l)) {
//        if (i == pos) s[i++] = '|';
//        s[i++] = '0' + getNext(l);
//        next(l);
//    }
//    if (i == pos) s[i++] = '|';
//    s[i++] = '\0';
//}
//
//// Check a particular function. The first argument is the name of the function.
//// The second and third arguments describes the list before and after the call.
//// The fourth, fifth and sixth arguments are an item to pass as an argument, an
//// item to expect as a result, and a boolean to expect as a result, if any.
//static bool check(char *op, char *lb, char *la, data x, data y, bool b) {
//    bool r = true;
//    data z = 0;
//    list *l = toList(lb);
//    if (strcmp(op, "startF") == 0) first(l);
//    else if (strcmp(op, "startB") == 0) last(l);
//    else if (strcmp(op, "endF") == 0) r = isLast(l);
//    else if (strcmp(op, "endB") == 0) r = isFirst(l);
//    else if (strcmp(op, "nextF") == 0) r = next(l);
//    else if (strcmp(op, "nextB") == 0) r = previous(l);
//    else if (strcmp(op, "insertF") == 0) insertNext(l, x);
//    else if (strcmp(op, "insertB") == 0) insertPrevious(l, x);
//    else if (strcmp(op, "getF") == 0) z = getNext(l);
//    else if (strcmp(op, "getB") == 0) z = getPrevious(l);
//    else if (strcmp(op, "setF") == 0) r = setNext(l, x);
//    else if (strcmp(op, "setB") == 0) r = setPrevious(l, x);
//    else if (strcmp(op, "deleteF") == 0) r = deleteNext(l);
//    else if (strcmp(op, "deleteB") == 0) r = deletePrevious(l);
//    else return false;
//    if (r != b || z != y) return false;
//    char s[100];
//    toString(l, s);
//    freeList(l);
//    return strcmp(s, la) == 0;
//}
//
//static void test0_1(){
//	list *l = createList(-1);
//	insertNext(l, 2);
//	assert(l->current->previous->value == 2);
//	assert(getPrevious(l) == 2);
//	freeList(l);
//}
//
//static void test0_2(){
//	list *l = createList(-1);
//	insertPrevious(l, 2);
//	char t[20];
//	toString(l,t);
//	//printf("Before: %s\n", t);
//	//assert(l->current->value == 2);
////	assert(getB(l) == 2);
//	first(l);
//	deleteNext(l);
//	//printf("Deleted:\n");
//
//	toString(l,t);
//	//printf("After: %s\n", t);
//	freeList(l);
//}
//
//// Test createList, freeList.
//static void test1() {
//    list *l = createList(-1);
//    assert(l != NULL);
//    freeList(l);
//}
//
//// Test insertNext, getPrevious
//static void test2() {
//    list *l = createList(-1);
//	char b[10];
//	toString(l, b);
//	//printf("The list before is: %s\n", b);
//    insertNext(l, 3);
//	char a[10];
//	toString(l, a);
//	//printf("The list after inserting 3 is: %s\n", a);
//    assert(getPrevious(l) == 3);
//    freeList(l);
//}
//
//// Test that startF and startB move to the beginning or end of the list.
//static void testStart() {
//    assert(check("startF", "|", "|", 0, 0, true));
//    assert(check("startF", "|37", "|37", 0, 0, true));
//    assert(check("startF", "3|7", "|37", 0, 0, true));
//    assert(check("startF", "37|", "|37", 0, 0, true));
//    assert(check("startB", "|", "|", 0, 0, true));
//    assert(check("startB", "|37", "37|", 0, 0, true));
//    assert(check("startB", "3|7", "37|", 0, 0, true));
//    assert(check("startB", "37|", "37|", 0, 0, true));
//}
//
//// Test that endF and endB detect the list being at the end or beginning.
//static void testEnd() {
//    assert(check("endF", "|", "|", 0, 0, true));
//    assert(check("endF", "|37", "|37", 0, 0, false));
//    assert(check("endF", "3|7", "3|7", 0, 0, false));
//    assert(check("endF", "37|", "37|", 0, 0, true));
//    assert(check("endB", "|", "|", 0, 0, true));
//    assert(check("endB", "|37", "|37", 0, 0, true));
//    assert(check("endB", "3|7", "3|7", 0, 0, false));
//    assert(check("endB", "37|", "37|", 0, 0, false));
//}
//
//// Test that nextF and nextB move forward or backward in the list.
//static void testNext() {
//    assert(check("nextF", "|", "|", 0, 0, false));
//    assert(check("nextF", "|37", "3|7", 0, 0, true));
//    assert(check("nextF", "3|7", "37|", 0, 0, true));
//    assert(check("nextF", "37|", "37|", 0, 0, false));
//    assert(check("nextB", "|", "|", 0, 0, false));
//    assert(check("nextB", "|37", "|37", 0, 0, false));
//    assert(check("nextB", "3|7", "|37", 0, 0, true));
//    assert(check("nextB", "37|", "3|7", 0, 0, true));
//}
//
//// Test that insertF and insertB insert behind the direction of motion.
//static void testInsert() {
//    assert(check("insertF", "|", "5|", 5, 0, true));
//    assert(check("insertF", "|37", "5|37", 5, 0, true));
//    assert(check("insertF", "3|7", "35|7", 5, 0, true));
//    assert(check("insertF", "37|", "375|", 5, 0, true));
//    assert(check("insertB", "|", "|5", 5, 0, true));
//    assert(check("insertB", "|37", "|537", 5, 0, true));
//    assert(check("insertB", "3|7", "3|57", 5, 0, true));
//    assert(check("insertB", "37|", "37|5", 5, 0, true));
//}
//
//// Test that getF and getB return the current item.
//static void testGet() {
//    assert(check("getF", "|", "|", 0, -1, true));
//    assert(check("getF", "|37", "|37", 0, 3, true));
//    assert(check("getF", "3|7", "3|7", 0, 7, true));
//    assert(check("getF", "37|", "37|", 0, -1, true));
//    assert(check("getB", "|", "|", 0, -1, true));
//    assert(check("getB", "|37", "|37", 0, -1, true));
//    assert(check("getB", "3|7", "3|7", 0, 3, true));
//    assert(check("getB", "37|", "37|", 0, 7, true));
//}
//
//// Test setF and setB. Check's second argument is the expected result.
//static void testSet() {
//    assert(check("setF", "|", "|", 5, 0, false));
//    assert(check("setF", "|37", "|57", 5, 0, true));
//    assert(check("setF", "3|7", "3|5", 5, 0, true));
//    assert(check("setF", "37|", "37|", 5, 0, false));
//    assert(check("setB", "|", "|", 5, 0, false));
//    assert(check("setB", "|37", "|37", 5, 0, false));
//    assert(check("setB", "3|7", "5|7", 5, 0, true));
//    assert(check("setB", "37|", "35|", 5, 0, true));
//}
//
//static void testDelete() {
//    assert(check("deleteF", "|", "|", 0, 0, false));
//    assert(check("deleteF", "|37", "|7", 0, 0, true));
//    assert(check("deleteF", "3|7", "3|", 0, 0, true));
//    assert(check("deleteF", "37|", "37|", 0, 0, false));
//    assert(check("deleteB", "|", "|", 0, 0, false));
//    assert(check("deleteB", "|37", "|37", 0, 0, false));
//    assert(check("deleteB", "3|7", "|7", 0, 0, true));
//    assert(check("deleteB", "37|", "3|", 0, 0, true));
//}
//
//int main() {
//	test0_1();
//	test0_2();
//    test1();
//    test2();
//    testStart();
//    testEnd();
//    testNext();
//    testInsert();
//    testGet();
//    testSet();
//    testDelete();
//    printf("Lists module OK\n");
//    return 0;
//}
//
//#endif
