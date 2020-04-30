/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "P6.h"

//int is_prime2( uint32_t x ) {
//  if ( !( x & 1 ) || ( x < 2 ) ) {
//    return ( x == 2 );
//  }
//
//  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
//    if( !( x % d ) ) {
//      return 0;
//    }
//  }
//
//  return 1;
//}
//
//void sleep (int t) {
//  for (int i = 0; i < 5000 * t; i++) {
//    int r = is_prime2( i ); 
//  }
//}

void main_P6() {

  //uint16_t fb[ 600 ][ 800 ] = lcdGetFb(); 
  //uint16_t fb[600][800] = 
  //lcdGetFb();
  //uint16_t (*fb)[800] = (uint16_t (*)[600][800])lcdGetFb(); 
  uint16_t (*fb)[800] = (uint16_t (*)[800]) lcdGetFb(); 
  
  //int *mouse_x_pointer = getMouseX();
  //int mouse_y = *getMouseY();
  
  write( STDOUT_FILENO, "c", 1 );
  for( int i = 0; i < 600; i++ ) {
    for( int j = 0; j < 800; j++ ) {
      fb[ i ][ j ] = 0x1F << ( ( i / 200 ) * 5 );
    }
  }
  square(100, 200, 100, fb);
  put_char('J', 100, 20, fb);
  put_char('A', 110, 20, fb);
  put_str("Hello", 110, 30, fb);
  draw();

  while(1) {
    write( STDOUT_FILENO, "d", 1 );
    //if (*mouse_x_pointer > 400) break;
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
