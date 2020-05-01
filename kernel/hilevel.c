/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

// Process table
list *procTab; 
// Current executing process and its priority
pcb_t* executing = NULL; 
int last_priority = 0;
// Process queue, lower priority number -> run first
priorityQueue *q; 

// Buffers for pipes
buffer buffers[MAX_PROCS];

// Stack for processes
extern uint32_t tos_P;
extern void main_console();
extern void main_P6();

// cusor vars
int mouse_pos_x = 400;
int mouse_pos_y = 300;
uint8_t mouse_left_state = 0;
bool hovering = false;

int number_of_procs = 0;

uint32_t procStackSize = 0x1000;

// Frame buffers
// fb is the actaul frame buffer
uint16_t fb[ 600 ][ 800 ];
// Next buffer is given to user process
// fb is set to fb_next_buffer during draw svc
uint16_t fb_next_buffer[ 600 ][ 800 ];
int current_buffer = 1;

void dispatch( ctx_t* ctx, pcb_t* next ) {
  //char prev_pid = '?', next_pid = '?';

  if( NULL != executing ) {
    memcpy( &executing->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
  }
  if( NULL != next ) {
    memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
  }

  executing = next;                           // update   executing process to P_{next}

  return;
}

void schedule( ctx_t* ctx ) {
    // Continue the current executing process if it is the highest priority and not terminated
    if (executing != NULL && executing->status != STATUS_TERMINATED && last_priority < ( (struct pqitem*) pqPeek(q) )->priority) {
      return;    
    }

    // If the process queue is empty then return
    if (q->size == 0) {
      return;
    }

    // Get next item off queue (queue is in priority order)
    pqitem *next_item = (struct pqitem*) pqPop(q);
    pcb_t *next_p = next_item->data;
    last_priority = next_item->priority;
  
    // If the next item is terminated loop until we find one which is not
    while (next_p->status == STATUS_TERMINATED) {
      next_item = (struct pqitem*) pqPop(q);
      next_p = next_item->data;
      last_priority = next_item->priority;
    }
    // Once the item has been used, free it
    free(next_item);
    
    // If the last process was NULL or has TERMINATED do not add it to the queue
    // Otherwise add it to the queue and set as ready
    if ( executing != NULL && executing->status != STATUS_TERMINATED ) { 
      // update execution status of previously executing process
      executing->status = STATUS_READY;         
      // push the process back onto the queue with the same priority
      pqPush (q, executing, last_priority);
    }

    dispatch( ctx, next_p );

    // update execution status of next executing process
    next_p->status = STATUS_EXECUTING;         

}

pcb_t* addProcess ( uint32_t pc ) {
  // Create new process
  pcb_t *newProc = malloc(sizeof(pcb_t));
  newProc->status     = STATUS_READY;
  newProc->pid        = number_of_procs;
  newProc->tos        = ( uint32_t )(&tos_P) - number_of_procs*procStackSize;
  newProc->ctx.pc     = pc;
  newProc->ctx.cpsr   = 0x50;
  newProc->ctx.sp     = newProc->tos;
  insertNext(procTab, newProc);

  // Incremenet the number of porcesses
  number_of_procs++;
  return newProc;
}

void hilevel_handler_rst(ctx_t* ctx ) {
  procTab =  newList();
 
  /* Set up timer */
  TIMER0->Timer1Load  = 0x00001000; // select period
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00300010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  // LCD Setup
  SYSCONF->CLCD      = 0x2CAC;     // per per Table 4.3 of datasheet
  LCD->LCDTiming0    = 0x1313A4C4; // per per Table 4.3 of datasheet
  LCD->LCDTiming1    = 0x0505F657; // per per Table 4.3 of datasheet
  LCD->LCDTiming2    = 0x071F1800; // per per Table 4.3 of datasheet

  // Set the framebuffer
  LCD->LCDUPBASE     = ( uint32_t )( &fb );

  LCD->LCDControl    = 0x00000020; // select TFT   display type
  LCD->LCDControl   |= 0x00000008; // select 16BPP display mode
  LCD->LCDControl   |= 0x00000800; // power-on LCD controller
  LCD->LCDControl   |= 0x00000001; // enable   LCD controller
  
  /* Configure the mechanism for interrupt handling by
   *
   * - configuring then enabling PS/2 controllers st. an interrupt is
   *   raised every time a byte is subsequently received,
   * - configuring GIC st. the selected interrupts are forwarded to the 
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */

  PS20->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
  PS20->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)
  PS21->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
  PS21->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)


  uint8_t ack;

        PL050_putc( PS20, 0xF4 );  // transmit PS/2 enable command
  ack = PL050_getc( PS20       );  // receive  PS/2 acknowledgement
        PL050_putc( PS21, 0xF4 );  // transmit PS/2 enable command
  ack = PL050_getc( PS21       );  // receive  PS/2 acknowledgement
 
  // Preset the frame buffers to plane white
  for( int i = 0; i < 600; i++ ) {
    for( int j = 0; j < 800; j++ ) {
      fb[ i ][ j ] = 0x7FFF;
      fb_next_buffer[ i ][ j ] = 0x7FFF;
    }
  }

  // Add the console to the processTab so it can be run after rst
  addProcess( ( uint32_t ) ( &main_console ) );

  // Init process queue as priority queue
  q = newPriorityQueue();

  // Add all processes in the procTab to the queue
  first(procTab);
  while (!isLast(procTab)) {
    int priority = 2;
    pqPush(q, (pcb_t*) getNext(procTab), priority);
    next(procTab);
  }

  schedule ( ctx );
  int_enable_irq();
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
  else if( id == GIC_SOURCE_PS20 ) {
    uint8_t x = PL050_getc( PS20 );

    PL011_putc( UART0, '0',                      true );  
    PL011_putc( UART0, '<',                      true ); 
    PL011_putc( UART0, itox( ( x >> 4 ) & 0xF ), true ); 
    PL011_putc( UART0, itox( ( x >> 0 ) & 0xF ), true ); 
    PL011_putc( UART0, '>',                      true ); 
  }
  else if( id == GIC_SOURCE_PS21 ) {
    // Undraw previous mouse pointer (a 20x20 grid)
    for( int i = 0; i < 20; i++ ) {
      for( int j = 0; j < 20; j++ ) {
        int pos_y = mouse_pos_y + i;
        if (pos_y > 599) pos_y = 599;
        else if (pos_y < 0) pos_y = 0;
        int pos_x = mouse_pos_x + j;
        if (pos_x > 799) pos_x = 799;
        else if (pos_x < 0) pos_x = 0;
        fb[pos_y][pos_x] = fb_next_buffer[pos_y][pos_x];
      }
    }
  
    // Get the mouse data
    uint8_t mouse_state = PL050_getc( PS21 );
    uint8_t move_x = PL050_getc( PS21 );
    uint8_t move_y = PL050_getc( PS21 );

    // Set if left mouse button is down 1->down 0->up
    mouse_left_state = mouse_state & 0x1;

    // Get the new mouse positions from the move
    // Sets new pos 
    // movement is 9 bit 2s compliments with msb stored in the state
    // Find value of move and add to global mouse position
    // If mouse outside of screen, set to boundry
    mouse_pos_x += move_x - 255* (mouse_state >> 4 & 0x1);
    if (mouse_pos_x < 0) mouse_pos_x = 0;
    else if (mouse_pos_x > 799) mouse_pos_x = 799;
    mouse_pos_y -= move_y - 255* (mouse_state >> 5 & 0x1);
    if (mouse_pos_y < 0) mouse_pos_y = 0;
    else if (mouse_pos_y > 599) mouse_pos_y = 599;
   
    drawMousePointer();
  }

  GICC0->EOIR = id;

  return;
}
 
void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) {  // Funciton parameter are r0 and r1 in lolevel
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
      // Add process to proc tab
      pcb_t* child = addProcess ( ctx->pc );
      // Copy current process
      memcpy ( &child->ctx, ctx , sizeof( ctx_t ));
      // Set stack pointer to same location and copy stack across
      uint32_t stackPointerDistance = executing->tos - ctx->sp;
      child->ctx.sp = child->tos - stackPointerDistance;
      memcpy ( (uint32_t*) child->ctx.sp , (uint32_t*) ctx->sp ,  stackPointerDistance );
      child->ctx.gpr[0] = 0;
      // Return child pid
      ctx->gpr[ 0 ] = child->pid;
      // Add child to queue
      pqPush (q, child, 2 );
      break;
    } // SVC when process finishes execution
    case 0x04 : { // 0x04 => Exit executing process
      PL011_putc( UART0, 'F', true );

      // Find the executing porcess in the procTab and remove
      first(procTab);
      while (!isLast(procTab)) {
        pcb_t* process = (pcb_t*) getNext(procTab);
        if (process == executing) {
          deleteNext(procTab);
          break;
        }
        next(procTab);
      }
      executing = NULL;
      schedule ( ctx );

      break;
    }
    case 0x05 : { // Execute
      // Set program counter of executing to start of new program
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
          // Find process in procTab
          first(procTab);
          while (!isLast(procTab)) {
            pcb_t* process = (pcb_t*) getNext(procTab);
            if (process->pid == pid) {
              // Delete from queue
              deleteItem(q, pid);
              // Delete from proctab
              deleteNext(procTab);
              break;
            }
            next(procTab);
          }
          break;
      }
      break;
    }
    case 0x08 : { // SYS_SEM_CREATE
      // Create a new semaphore in heap
      uint32_t* semaphore = malloc(sizeof(uint32_t));
      // Set its value to that in r0
      *semaphore = ctx->gpr[0];
      // Return pointer to semaphore
      ctx->gpr[0] = (uint32_t) semaphore;
      break;
    }
    case 0x09 : { // SYS_SEM_DESTROY
      // Free the semaphore
      uint32_t* semaphore = (uint32_t*) ctx->gpr[0];
      free(semaphore);
      break;
    }
    
    case 0x0A : { // SYS_PIPE_CREATE
      buffer b;
      for (int i = 0; i < MAX_PROCS; i++) {
        // Add a new buffer to the buffers array of given length
        if (buffers[i].inUse == false) {
          b = buffers[i];
          b.inUse = true;
          b.length = ctx->gpr[0];
          b.data = newQueue();
          ctx->gpr[0] = i; // Return index of buffer
          break;
        }
      }
      break;
    }
    case 0x0B : { // SYS_PIPE_DESTROY
      // Remove from array and free buffer
      buffer b = buffers[ctx->gpr[0]];
      freeQueue(b.data);
      b.inUse = false;
      break;
    }
    case 0x0C : { // SYS_PIPE_SEND
      // Add given value to buffer queue
      char bId = ctx->gpr[0];
      char d   = (char)ctx->gpr[1];
      push( buffers[bId].data, (qdata) &d );
      break;
    }
    case 0x0D : { // SYS_PIPE_RECEIVE
      // Take value off buffer queue
      char bId = ctx->gpr[0];
      char d = *(char*) pop( buffers[bId].data );
      ctx->gpr[0] = d;
      break;
    }
    case 0x0E : { // LCD_CREATE
      // Return pointer to new fb which is drawn in LCD_DRAW
      ctx->gpr[0] = (uint32_t)&fb_next_buffer[0][0];
      break;
    }
    case 0x0F : { // LCD_DRAW
      // Set current fb equal to the fb given to programs
      for( int i = 0; i < 600; i++ ) {
        for( int j = 0; j < 800; j++ ) {
          fb[i][j] = fb_next_buffer[i][j];
        }
      }
      break;
    }
    case 0x10 : { // LCD_MOUSE_X
      // Return pointer to global mouse x pos
      ctx->gpr[0] = (uint32_t)&mouse_pos_x;
      break;
    }
    case 0x11 : { // LCD_MOUSE_Y
      // Return pointer to global mouse y pos
      ctx->gpr[0] = (uint32_t)&mouse_pos_y;
      break;
    }
    case 0x12 : { // LCD_MOUSE_LEFT
      // Return pointer to global left mouse button state
      // 1-> pressed 0-> unpressed
      ctx->gpr[0] = (uint32_t)&mouse_left_state;
      break;
    }
    case 0x13 : { // LCD_HOVER
      // Set whether the mouse is hovering
      // If so pointer can change
      // Used to indicate clickable
      hovering = (bool)ctx->gpr[0];
      break;
    }
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}

// Helper function to ensure drawn in fb range
void drawPixel(int y, int x, uint16_t color) {
  if (x < 800 && y < 600) fb[y][x] = color;
}

void drawMousePointer() {
    uint32_t fillColor = hovering ? 0x001F : 0x7FFF;
    // Draw Left line
    for ( int i = 0; i < 17; i++ ) {
      drawPixel(mouse_pos_y+i, mouse_pos_x, 0x0);
    }
    // Right diagonal line
    for ( int i = 0; i < 12; i++ ) {
      drawPixel(mouse_pos_y+i, mouse_pos_x+i, 0x0);
    }
    // Fill in above section 
    for ( int i = 0; i < 12; i++ ) {
      for (int j = i-1; j > 0; j--) {
        drawPixel(mouse_pos_y+i, mouse_pos_x+j, fillColor);
      }
    }
    // Bottom up diagonal
    for ( int i = 0; i < 4; i++ ) {
      drawPixel(16+mouse_pos_y-i,1+mouse_pos_x+i,0x0);
    }
    // Fill in above section 
    for ( int i = 12; i < 16; i++ ) {
      for (int j = 4-(i-12); j > 0; j--) {
        drawPixel(mouse_pos_y+i, mouse_pos_x+j, fillColor);
      }
    }
    // Middle straight
    for ( int i = 0; i < 5; i++ ) {
      drawPixel(12+mouse_pos_y,11+mouse_pos_x-i, 0x0);
    }
    // Tail
    // Tail Left
    drawPixel(13+mouse_pos_y, 4+mouse_pos_x, 0x0);
    drawPixel(14+mouse_pos_y, 4+mouse_pos_x, 0x0);
    drawPixel(15+mouse_pos_y, 5+mouse_pos_x, 0x0);
    drawPixel(16+mouse_pos_y, 5+mouse_pos_x, 0x0);
    drawPixel(17+mouse_pos_y, 6+mouse_pos_x, 0x0);
    drawPixel(18+mouse_pos_y, 6+mouse_pos_x, 0x0);
    // Fill in above Seciton
    for ( int i = 0; i < 2; i++ ) {
      for ( int j = 0; j < 3; j++ ) {
        drawPixel(11+mouse_pos_y+i, 4+mouse_pos_x+j, fillColor);
        drawPixel(13+mouse_pos_y+i, 5+mouse_pos_x+j, fillColor);
        drawPixel(15+mouse_pos_y+i, 6+mouse_pos_x+j, fillColor);
        drawPixel(17+mouse_pos_y+i, 7+mouse_pos_x+j, fillColor);
      }
    }

    // Tail Right
    drawPixel(13+mouse_pos_y, 4+mouse_pos_x+3, 0x0);
    drawPixel(14+mouse_pos_y, 4+mouse_pos_x+3, 0x0);
    drawPixel(15+mouse_pos_y, 5+mouse_pos_x+3, 0x0);
    drawPixel(16+mouse_pos_y, 5+mouse_pos_x+3, 0x0);
    drawPixel(17+mouse_pos_y, 6+mouse_pos_x+3, 0x0);
    drawPixel(18+mouse_pos_y, 6+mouse_pos_x+3, 0x0);
    // Bottom
    drawPixel(19+mouse_pos_y, 7+mouse_pos_x, 0x0);
    drawPixel(19+mouse_pos_y, 8+mouse_pos_x, 0x0);
}

// 2nd Mouse pointer for clickable
//void drawMousePointer() {
//  fillColor = 0x7FFF;
//  outlineColor = 0x0;
//
//  //BottomLine
//  for ( int i = 0; i < 10; i++ ){
//    drawPixel(15+mouse_pos_y, (-1)+mouse_pos_x, uint16_t color);
//  }
//
//}
