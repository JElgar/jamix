/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "P6.h"

void main_P6() {
  while( 1 ) {
    write( STDOUT_FILENO, "P6", 2 );

    uint32_t lo = 1 <<  4;
    uint32_t hi = 1 <<  8;

    for( uint32_t x = lo; x < hi; x++ ) {
      for( uint32_t y = lo; y < hi; y++ ) {
        uint32_t r = gcd( x, y ); 
      }
    }
  }

  exit( EXIT_SUCCESS );
}
