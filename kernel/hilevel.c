/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

list *procTab;
//pcb_t procTab[ MAX_PROCS ]; 
pcb_t* executing = NULL; priorityQueue *q; 

// Buffers for pipes
buffer buffers[MAX_PROCS];

// Stack for porcesses
extern uint32_t tos_P;
extern void main_console();
extern void main_P6();

// cusor vars
int mouse_pos_x = 400;
int mouse_pos_y = 300;
uint8_t mouse_left_state = 0;

int last_priority = 0;
int number_of_procs = 0;

uint32_t procStackSize = 0x1000;

// Frame buffer
uint16_t fb[ 600 ][ 800 ];
uint16_t fb_next_buffer[ 600 ][ 800 ];
//uint16_t fb2[ 600 ][ 800 ];
//uint16_t *fb_active[ 600 ][ 800 ];
int current_buffer = 1;

// Cursor
void dispatch( ctx_t* ctx, pcb_t* next ) {
  //char prev_pid = '?', next_pid = '?';

  if( NULL != executing ) {
    memcpy( &executing->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
  //   prev_pid = '0' + executing->pid;
  }
  if( NULL != next ) {
    memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
  //  next_pid = '0' + next->pid;
  }

    //PL011_putc( UART0, '[',      true );
    //PL011_putc( UART0, prev_pid, true );
    //PL011_putc( UART0, '-',      true );
    //PL011_putc( UART0, '>',      true );
    //PL011_putc( UART0, next_pid, true );
    //PL011_putc( UART0, ']',      true );

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
    free(next_item);
    while (next_p->status == STATUS_TERMINATED) {
      next_item = (struct pqitem*) pqPop(q);
      next_p = next_item->data;
      free(next_item);
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
  //memset( &procTab[ number_of_procs ], 0, sizeof( pcb_t ) ); 
  pcb_t *newProc = malloc(sizeof(pcb_t));
  newProc->status     = STATUS_READY;
  newProc->pid        = number_of_procs;
  newProc->tos        = ( uint32_t )(&tos_P) - number_of_procs*procStackSize;
  newProc->ctx.pc     = pc;
  newProc->ctx.cpsr   = 0x50;
  newProc->ctx.sp     = newProc->tos;
  insertNext(procTab, newProc);

  number_of_procs++;
  return newProc;
}

void hilevel_handler_rst(ctx_t* ctx ) {
  procTab =  newList();
 
  /* Set up timer */
  //TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
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

  LCD->LCDUPBASE     = ( uint32_t )( &fb );
  //LCD->LCDLPBASE     = ( uint32_t )( &fblp );

  LCD->LCDControl    = 0x00000020; // select TFT   display type
  LCD->LCDControl   |= 0x00000008; // select 16BPP display mode
  LCD->LCDControl   |= 0x00000800; // power-on LCD controller
  LCD->LCDControl   |= 0x00000001; // enable   LCD controller
  //LCD->LCDControl   |= 0x10000000; // enable dual panel
  
  // Enable cursor
  LCD->ClcdCrsrPalette0 = 0x0;
  LCD->ClcdCrsrPalette1 = 0xFFF;
  //LCD->ClcdCrsrXY = 0x0A0A;
  for (int i = 0; i < 64; i++) {
    LCD->ClcdCrsrImage[i] = 0x0;
  }
  LCD->ClcdCrsrCtrl = 0x1;

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
  
  for( int i = 0; i < 600; i++ ) {
    for( int j = 0; j < 800; j++ ) {
      fb[ i ][ j ] = 0x7FFF;
      //fb2[ i ][ j ] = 0x0;
      //fb_buffer_1[ i ][ j ] = 0x7FFF;
      //fb_buffer_2[ i ][ j ] = 0x7FFF;
      fb_next_buffer[ i ][ j ] = 0x7FFF;
    }
  }
  //for( int i = 0; i < 600; i++ ) {
  //  for( int j = 0; j < 800; j++ ) {
  //    fb[ i ][ j ] = 0x1F << ( ( i / 200 ) * 5 );
  //  }
  //}

  addProcess( ( uint32_t ) ( &main_console ) );
  addProcess( ( uint32_t ) ( &main_P6 ) );
  
  q = newPriorityQueue();
  //for ( int i = 0; i < number_of_procs; i++ ) {
  //  int priority = 2;
  //  pqPush(q, &procTab[ i ], priority);
  //}

  first(procTab);
  while (!isLast(procTab)) {
    int priority = 2;
    //pqPush(q, (struct pcb_t*) getNext(procTab), priority);
    pqPush(q, (pcb_t*) getNext(procTab), priority);
    next(procTab);
  }

  /* Once the PCBs are initialised, we arbitrarily select the 0-th PCB to be 
   * executed: there is no need to preserve the execution context, since it 
   * is invalid on reset (i.e., no process was previously executing).
   */

  // dispatch( ctx, NULL, &procTab[ 0 ] );
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
    // Undraw previous mouse pointer
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
    uint8_t mouse_state = PL050_getc( PS21 );
    uint8_t move_x = PL050_getc( PS21 );
    uint8_t move_y = PL050_getc( PS21 );

    mouse_left_state = mouse_state & 0x1;

    mouse_pos_x += move_x - 255* (mouse_state >> 4 & 0x1);
    if (mouse_pos_x < 0) mouse_pos_x = 0;
    else if (mouse_pos_x > 799) mouse_pos_x = 799;
    mouse_pos_y -= move_y - 255* (mouse_state >> 5 & 0x1);
    if (mouse_pos_y < 0) mouse_pos_y = 0;
    else if (mouse_pos_y > 599) mouse_pos_y = 599;
    
    drawMousePointer();
  }
  flip();


  //for( int i = 0; i < 600; i++ ) {
  //  for( int j = 0; j < 800; j++ ) {
  //    fb[ i ][ j ] = fb_next_buffer[ i ][ j ];
  //  }
  //}
  //if (current_buffer == 1) {
  //  fb[0][0] = &fb_buffer_2[0][0];
  //  for( int i = 0; i < 10; i++ ) {
  //    for( int j = 0; j < 10; j++ ) {
  //      fb[mouse_pos_y + i][mouse_pos_x + j] = 0x0;
  //    }
  //  }
  //  current_buffer = 2;
  //  fb_buffer_1[0][0] = fb_next_buffer;
  //}
  //else if (current_buffer == 2) {
  //  fb[0][0] = &fb_buffer_1[0][0];
  //  for( int i = 0; i < 10; i++ ) {
  //    for( int j = 0; j < 10; j++ ) {
  //      fb[mouse_pos_y + i][mouse_pos_x + j] = 0x0;
  //    }
  //  }
  //  current_buffer = 1;
  //  fb_buffer_2[0][0] = fb_next_buffer;
  //}
  


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
    case 0x04 : { // 0x04 => Exit executing process
      PL011_putc( UART0, 'F', true );

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
          first(procTab);
          while (!isLast(procTab)) {
            pcb_t* process = (pcb_t*) getNext(procTab);
            if (process->pid == pid) {
              deleteItem(q, pid);
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
      uint32_t* semaphore = malloc(sizeof(uint32_t));
      *semaphore = ctx->gpr[0];
      ctx->gpr[0] = (uint32_t) semaphore;
      break;
    }
    case 0x09 : { // SYS_SEM_DESTROY
      uint32_t* semaphore = (uint32_t*) ctx->gpr[0];
      free(semaphore);
      break;
    }
    
    case 0x0A : { // SYS_PIPE_CREATE
      buffer b;
      for (int i = 0; i < MAX_PROCS; i++) {
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
      buffer b = buffers[ctx->gpr[0]];
      freeQueue(b.data);
      b.inUse = false;
      break;
    }
    case 0x0C : { // SYS_PIPE_SEND
      char bId = ctx->gpr[0];
      char d   = ctx->gpr[1];
      push( buffers[bId].data, (qdata) d );
      break;
    }
    case 0x0D : { // SYS_PIPE_RECEIVE
      char bId = ctx->gpr[0];
      char d = (char) pop( buffers[bId].data );
      ctx->gpr[0] = d;
      break;
    }
    //case 0x0E : { // LCD_COLOR
    //  for( int i = 0; i < 600; i++ ) {
    //    for( int j = 0; j < 800; j++ ) {
    //      fb[ i ][ j ] = 0x1F << ( ( i / 200 ) * 5 );
    //    }
    //  }
    //}
    //case 0x0F : { // LCD_WHITE
    //  for( int i = 0; i < 600; i++ ) {
    //    for( int j = 0; j < 800; j++ ) {
    //      fb[ i ][ j ] = 0x7FFF;
    //    }
    //  }
    //  break;
    //}
    case 0x0E : { // LCD_CREATE
      ctx->gpr[0] = (uint32_t)&fb_next_buffer[0][0];
      break;
    }
    case 0x0F : { // LCD_DRAW
      for( int i = 0; i < 600; i++ ) {
        for( int j = 0; j < 800; j++ ) {
          fb[i][j] = fb_next_buffer[i][j];
        }
      }
      break;
    }
    case 0x10 : { // LCD_MOUSE_X
      ctx->gpr[0] = (uint32_t)&mouse_pos_x;
      break;
    }
    case 0x11 : { // LCD_MOUSE_Y
      ctx->gpr[0] = (uint32_t)&mouse_pos_y;
      break;
    }
    case 0x12 : { // LCD_MOUSE_LEFT
      ctx->gpr[0] = (uint32_t)&mouse_left_state;
      break;
    }
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}

void flip() {
  //if(current_buffer == 1) {
  //  LCD->LCDUPBASE = (uint32_t) (&fb2);
  //  current_buffer = 2;
  //}
  //else {
  //  LCD->LCDUPBASE = (uint32_t) (&fb);
  //  current_buffer = 1;
  //}
}

void drawPixel(int y, int x, uint16_t color) {
  if (x < 800 && y < 600) fb[y][x] = color;
}

void drawMousePointer() {
    //for( int i = 0; i < 10; i++ ) {
    //  for( int j = 0; j < 10; j++ ) {
    //    int pos_y = mouse_pos_y + i;
    //    if (pos_y > 599) pos_y = 599;
    //    else if (pos_y < 0) pos_y = 0;
    //    int pos_x = mouse_pos_x + j;
    //    if (pos_x > 799) pos_x = 799;
    //    else if (pos_x < 0) pos_x = 0;
    //    fb[pos_y][pos_x] = 0x0;
    //  }
    //}
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
        drawPixel(mouse_pos_y+i, mouse_pos_x+j, 0x7FFF);
      }
    }
    // Bottom up diagonal
    for ( int i = 0; i < 4; i++ ) {
      drawPixel(16+mouse_pos_y-i,1+mouse_pos_x+i,0x0);
    }
    // Fill in above section 
    for ( int i = 12; i < 16; i++ ) {
      for (int j = 4-(i-12); j > 0; j--) {
        drawPixel(mouse_pos_y+i, mouse_pos_x+j, 0x7FFF);
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
        drawPixel(11+mouse_pos_y+i, 4+mouse_pos_x+j, 0x7FFF);
        drawPixel(13+mouse_pos_y+i, 5+mouse_pos_x+j, 0x7FFF);
        drawPixel(15+mouse_pos_y+i, 6+mouse_pos_x+j, 0x7FFF);
        drawPixel(17+mouse_pos_y+i, 7+mouse_pos_x+j, 0x7FFF);
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
