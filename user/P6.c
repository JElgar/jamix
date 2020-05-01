/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "P6.h"

extern void main_P3(); 

void main_P6() {

  // Get program frame buffer
  uint16_t (*fb)[800] = (uint16_t (*)[800]) lcdGetFb(); 
  
  // Get global mouse pos and left mouse button state
  int *mouse_x_pointer = getMouseX();
  int *mouse_y_pointer = getMouseY();
  uint8_t *mouse_left_state = getMouseLeft();
 
  write( STDOUT_FILENO, "c", 1 );
  // Draw background
  for( int i = 0; i < 600; i++ ) {
    for( int j = 0; j < 800; j++ ) {
      fb[ i ][ j ] = 0x1F << ( ( i / 200 ) * 5 );
    }
  }
  
  // Draw square and some text
  square(100, 200, 100, 0x0, fb);
  put_char('J', 100, 20, 0x0, fb);
  put_char('E', 110, 20, 0x0, fb);
  put_str("Welcome to Jamos", 110, 30, 0x0, fb);

  int pid;
  int *pid_pointer = &pid;
  *pid_pointer = 2;
  // Create and draw two buttons
  buttonStruct p3b = {
    200,
    300,
    100,
    0x7FFF,
    &main_P3,
    false,
    false
  };
  button(p3b, "Click to Execute P3", 0x7FFF, fb);
  buttonStruct tb = {
    450,
    300,
    100,
    0x7FFF,
    &main_P3,
    false,
    false
  };
  button(tb, "Click to Terminate pid" , 0x00, fb);
  //put_char((*pid_pointer + '0'), 570, 350, fb);

  // Draw program framebuffer into actaul framebuffer
  draw();

  while(1) {
    // Handle click on execute P3 button
    handleExecClick(*mouse_x_pointer, *mouse_y_pointer, *mouse_left_state, &p3b, pid_pointer, fb);
    handleTerminateClick(*mouse_x_pointer, *mouse_y_pointer, *mouse_left_state, pid_pointer, &tb, fb);
  }
  exit( EXIT_SUCCESS );
}
