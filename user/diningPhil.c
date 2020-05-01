#include "diningPhil.h"

int numberOfPhils = 16;

// When using this chill function, it was optimised out
void chill(int i) {
  for(volatile int x = 0; x < 1000000 * i; x++){
    asm(""); // Prevents optimising out
  }
  return;
}


void phil(uint32_t* left, uint32_t* right, uint32_t* waiter, int id) {
  write( STDOUT_FILENO, "Phil", 4 );
  while(1) {
    // Do some thinking
    for(volatile int x = 0; x < 1000000; x++){
      asm(""); // Prevents optimising out
    }
 
    // Wait till the waiter is free and then claim
    sem_wait(waiter);
    // Take both left and right cutlery (lock semaphores)
    sem_wait(left);
    sem_wait(right);
    // Set the waiter as ready
    sem_post(waiter);
 
    // Show which philosipher is eating
    char *r;
    itoa(r, id);
    write( STDOUT_FILENO, r, 1 );
    write( STDOUT_FILENO, "munched", 8);

    // Do eating
    chill(10);
    for(volatile int x = 0; x < 1000000; x++){
      asm(""); // Prevents optimising out
    }

    // Put cutlery back down (unlock semaphore)
    sem_post(left);
    sem_post(right);
  }
}

int main_philling()
{
  write( STDOUT_FILENO, "Phil", 4 );
  uint32_t* cutlery[numberOfPhils];
  // Creates a waiter semaphore so only one philosipher can pick up at a time
  uint32_t* waiter = createSemaphore(1);
 
  // Create sempahore for each philosipher's left piece of cutlery
  for (int i = 0; i < numberOfPhils; i++) {
    write( STDOUT_FILENO, "P", 1 );
    cutlery[i] = createSemaphore(1);
  }
  for (int i = 0; i < numberOfPhils; i++) {
    write( STDOUT_FILENO, "Q", 1 );
    int ip1 = i + 1 > numberOfPhils ? 0 : i+1;
    // Fork and if i am the child
    if (fork() == 0) {
      phil(cutlery[i], cutlery[ip1], waiter, i); 
    }
  }
  exit(EXIT_SUCCESS);
}
