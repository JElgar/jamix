#include "diningPhil.h"

int numberOfPhils = 16;

void chill(int i) {
  for(int x = 0; x < i*100000; i++){
  
  }
  return;
}


void phil(uint32_t* left, uint32_t* right, uint32_t* waiter, int id) {
  write( STDOUT_FILENO, "Phil", 4 );
  while(1) {
    chill(10);
  
    sem_wait(waiter);
    sem_wait(left);
    sem_wait(right);
    sem_post(waiter);
  
    char *r;
    itoa(r, id);
    write( STDOUT_FILENO, r, 1 );
    write( STDOUT_FILENO, "munched", 8);

    chill(10);

    sem_post(left);
    sem_post(right);
  }
}

int main_philling()
{
  write( STDOUT_FILENO, "Phil", 4 );
  uint32_t* cutlery[numberOfPhils];
  uint32_t* waiter = createSemaphore(1);
  
  for (int i = 0; i < numberOfPhils; i++) {
    write( STDOUT_FILENO, "P", 1 );
    cutlery[i] = createSemaphore(1);
  }
  for (int i = 0; i < numberOfPhils; i++) {
    write( STDOUT_FILENO, "Q", 1 );
    int ip1 = i + 1 > numberOfPhils ? 0 : i+1;
    // If i am the child
    if (fork() == 0) {
      phil(cutlery[i], cutlery[ip1], waiter, i); 
    }
  }
  exit(EXIT_SUCCESS);
}
