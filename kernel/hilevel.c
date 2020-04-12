/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#include "queue/priorityQueue.h"

pcb_t procTab[ MAX_PROCS ]; pcb_t* executing = NULL; priorityQueue *q; 

// Stack for porcesses
extern uint32_t tos_P;
extern void main_console();

int last_priority = 0;
int number_of_procs = 0;

uint32_t procStackSize = 0x1000;

void dispatch( ctx_t* ctx, pcb_t* next ) {
  char prev_pid = '?', next_pid = '?';

  if( NULL != executing ) {
    memcpy( &executing->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
    prev_pid = '0' + executing->pid;
  }
  if( NULL != next ) {
    memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
    next_pid = '0' + next->pid;
  }

    PL011_putc( UART0, '[',      true );
    PL011_putc( UART0, prev_pid, true );
    PL011_putc( UART0, '-',      true );
    PL011_putc( UART0, '>',      true );
    PL011_putc( UART0, next_pid, true );
    PL011_putc( UART0, ']',      true );

    executing = next;                           // update   executing process to P_{next}

  return;
}

void schedule( ctx_t* ctx ) {
    if (executing != NULL && executing->status != STATUS_TERMINATED && last_priority < ( (struct pqitem*) pqPeek(q) )->priority) {
      return;    
    }
    if (q->size == 0) {
      return;
    }
    pqitem *next_item = (struct pqitem*) pqPop(q);
    pcb_t *next_p = next_item->data;
    while (next_p->status == STATUS_TERMINATED) {
      next_item = (struct pqitem*) pqPop(q);
      next_p = next_item->data;
    }
    
    // If the last process was NULL or has TERMINATED do not add it to the queue
    // Otherwise add it to the queue and set as ready
    if ( executing != NULL && executing->status != STATUS_TERMINATED ) { 
      executing->status = STATUS_READY;         // update   execution status  of P_2
      pqPush (q, executing, last_priority);
    }

    dispatch( ctx, next_p );

    next_p->status = STATUS_EXECUTING;         // update   execution status  of P_2

    last_priority = next_item->priority;
}

pcb_t* addProcess ( uint32_t pc ) {
  memset( &procTab[ number_of_procs ], 0, sizeof( pcb_t ) ); 
  pcb_t *newProc = &procTab [ number_of_procs ];
  newProc->status     = STATUS_READY;
  newProc->pid        = number_of_procs;
  newProc->tos        = ( uint32_t )(&tos_P) - number_of_procs*procStackSize;
  newProc->ctx.pc     = pc;
  newProc->ctx.cpsr   = 0x50;
  newProc->ctx.sp     = newProc->tos;

  number_of_procs++;
  return newProc;
}

void hilevel_handler_rst(ctx_t* ctx ) {
 
  /* Set up timer */
  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  int_enable_irq();
  
  addProcess( ( uint32_t ) ( &main_console ) );
  
  q = newPriorityQueue();
  for ( int i = 0; i < number_of_procs; i++ ) {
    int priority = 2;
    pqPush(q, &procTab[ i ], priority);
  }

  /* Once the PCBs are initialised, we arbitrarily select the 0-th PCB to be 
   * executed: there is no need to preserve the execution context, since it 
   * is invalid on reset (i.e., no process was previously executing).
   */

  // dispatch( ctx, NULL, &procTab[ 0 ] );
  schedule ( ctx );

  return;
}

void hilevel_handler_irq( ctx_t* ctx ) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    // call the scheduler
    PL011_putc( UART0, 'T', true ); 
    schedule(ctx);
    TIMER0->Timer1IntClr = 0x01;
  }

  // Step 5: write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;

  return;
}

/* theses are r0 and r1 in the lolelelv.c */
void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) { 
  /* Based on the identifier (i.e., the immediate operand) extracted from the
   * svc instruction, 
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call, then
   * - write any return value back to preserved usr mode registers.
   */
  
  switch( id ) {
    case 0x00 : { // 0x00 => yield()
      schedule( ctx );

      break;
    }

    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      char*  x = ( char* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }
      
      ctx->gpr[ 0 ] = n;

      break;
    }

    case 0x03 : { // Fork
      //PL011_putc( UART0, 'F', true );

      pcb_t* child = addProcess ( ctx->pc ); // Add proccess to proc tab
      memcpy ( &child->ctx, ctx , sizeof( ctx_t ));
      uint32_t stackPointerDistance = executing->tos - ctx->sp;
      child->ctx.sp = child->tos - stackPointerDistance;
      memcpy ( (uint32_t*) child->ctx.sp , (uint32_t*) ctx->sp ,  stackPointerDistance );
      child->ctx.gpr[0] = 0;
      ctx->gpr[ 0 ] = child->pid;
      pqPush (q, child, 2 );
      break;
    } // SVC when process finishes execution
    case 0x04 : { // 0x04 => Set executing process to TERMINATED
     
      // TODO
      // Delete from proc tab
      // Execute = null
      PL011_putc( UART0, 'F', true );
      executing->status = STATUS_TERMINATED;
      schedule ( ctx );

      break;
    }
    case 0x05 : { // Execute
      ctx->pc = ctx->gpr[ 0 ];
      ctx->sp = executing->tos;
      break;
    }
    case 0x06 : { // Kill
      uint32_t pid = ctx->gpr[ 0 ];
      uint32_t sig = ctx->gpr[ 1 ];
      switch (sig) {
        case 0x0: //SIG_TERM 
        case 0x1: //SIG_QUIT
          for (int i = 0; i < number_of_procs; i++) {
            if (procTab[i].pid == pid) {
              procTab[i].status = STATUS_TERMINATED; 
              deleteItem(q, pid);
              break;
            }
          }
          break;
      }
      break;
    }


    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
