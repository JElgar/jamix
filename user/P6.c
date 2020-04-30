/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "P6.h"

extern void main_philling(); 
extern void main_P3(); 

void main_P6() {

  uint16_t (*fb)[800] = (uint16_t (*)[800]) lcdGetFb(); 
  
  int *mouse_x_pointer = getMouseX();
  int *mouse_y_pointer = getMouseY();
  uint8_t *mouse_left_state = getMouseLeft();
  //int mouse_y = *getMouseY();
  
  write( STDOUT_FILENO, "c", 1 );
  for( int i = 0; i < 600; i++ ) {
    for( int j = 0; j < 800; j++ ) {
      fb[ i ][ j ] = 0x1F << ( ( i / 200 ) * 5 );
    }
  }
  square(100, 200, 100, 0x0, fb);
  put_char('J', 100, 20, fb);
  put_char('A', 110, 20, fb);
  put_str("Hello", 110, 30, fb);
  buttonStruct p3b = {
    200,
    300,
    100,
    0x7FFF,
    &main_P3
  };
  button(p3b, "Click to Execute P3", fb);
  buttonStruct tb = {
    450,
    300,
    100,
    0x7FFF,
    &main_P3
  };
  button(tb, "Click to Terminate pid 2", fb);
  draw();

  while(1) {
    handleExecClick(*mouse_x_pointer, *mouse_y_pointer, *mouse_left_state, p3b, fb);
    handleTerminateClick(*mouse_x_pointer, *mouse_y_pointer, *mouse_left_state, 1, tb, fb);
  }
  //while(1) {
  //  if (*mouse_x > 200 && *mouse_x < 400) {
  //    if (*mouse_y > 200 && *mouse_y < 400) {
  //      break;
  //    }
  //  }
  //}
  exit( EXIT_SUCCESS );
}
