/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

#include "queue/priorityQueue.h"
pcb_t procTab[ MAX_PROCS ]; pcb_t* executing = NULL; priorityQueue *q; 
extern void     main_P3(); 
//extern uint32_t tos_P3;
extern void     main_P4(); 
//extern uint32_t tos_P4;
extern void     main_P5(); 
//extern uint32_t tos_P5;
extern uint32_t tos_P;

int last_priority = 0;

void dispatch( ctx_t* ctx, pcb_t* prev, pcb_t* next ) {
  char prev_pid = '?', next_pid = '?';

  if( NULL != prev ) {
    memcpy( &prev->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
    prev_pid = '0' + prev->pid;
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
    // DO i need to add p5 back to the queue before swapping to the next item? I think now is too late
    pcb_t *last_p = executing;
    if (last_p != NULL && last_p->status != STATUS_TERMINATED && last_priority < ((struct pqitem*) pqPeek(q))->priority) {
      return;    
    }
    pqitem *next_item = (struct pqitem*) pqPop(q);
    pcb_t *next_p = next_item->data;
    dispatch( ctx, last_p, next_p );

    next_p->status = STATUS_EXECUTING;         // update   execution status  of P_2

    // If the last process was NULL or has TERMINATED do not add it to the queue
    // Otherwise add it to the queue and set as ready
    if ( last_p != NULL && last_p->status != STATUS_TERMINATED ) { 
      last_p->status = STATUS_READY;         // update   execution status  of P_2
      pqPush (q, last_p, last_priority);
    }
    last_priority = next_item->priority;

    return;

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
  
  // Process Table -> 0 -> p3 1 -> p4
  memset( &procTab[ 0 ], 0, sizeof( pcb_t ) ); // initialise 0-th PCB = P_1
  procTab[ 0 ].pid      = 1;
  procTab[ 0 ].status   = STATUS_READY;
  //procTab[ 0 ].tos      = ( uint32_t )( &tos_P3  );
  procTab[ 0 ].ctx.cpsr = 0x50;
  procTab[ 0 ].ctx.pc   = ( uint32_t )( &main_P3 );
  //procTab[ 0 ].ctx.sp   = procTab[ 0 ].tos;

  memset( &procTab[ 1 ], 0, sizeof( pcb_t ) ); // initialise 1-st PCB = P_2
  procTab[ 1 ].pid      = 2;
  procTab[ 1 ].status   = STATUS_READY;
  //procTab[ 1 ].tos      = ( uint32_t )( &tos_P4  );
  procTab[ 1 ].ctx.cpsr = 0x50;
  procTab[ 1 ].ctx.pc   = ( uint32_t )( &main_P4 );
  //procTab[ 1 ].ctx.sp   = procTab[ 1 ].tos;
  
  memset( &procTab[ 2 ], 0, sizeof( pcb_t ) ); // initialise 1-st PCB = P_2
  procTab[ 2 ].pid      = 3;
  procTab[ 2 ].status   = STATUS_READY;
  //procTab[ 2 ].tos      = ( uint32_t )( &tos_P5  );
  procTab[ 2 ].ctx.cpsr = 0x50;
  procTab[ 2 ].ctx.pc   = ( uint32_t )( &main_P5 );
  //procTab[ 2 ].ctx.sp   = procTab[ 2 ].tos;

  // TODO Wait a sec do i need to be taking away
  uint32_t current = ( uint32_t ) &tos_P;
  uint32_t size = 0x00003000 / MAX_PROCS;
  for ( int i = 0; i < MAX_PROCS; i++ ) {
    procTab[i].tos = ( uint32_t )( current );
    procTab[i].ctx.sp = procTab[i].tos;
    current -= size;
  }

  q = newPriorityQueue();
  for ( int i = 0; i < MAX_PROCS; i++ ) {
    int priority = i == 2 ? 1 : 2;
    priority = i == 1 ? 3 : priority;
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
    PL011_putc( UART0, 'T', true ); TIMER0->Timer1IntClr = 0x01;
    schedule(ctx);
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
    
    case 0x04 : { // 0x04 => Set executing process to TERMINATED

      PL011_putc( UART0, 'F', true );
      //procTab[ 2 ].status = STATUS_TERMINATED;
      executing->status = STATUS_TERMINATED;
      schedule ( ctx );

      break;
    }

    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
