/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "P6.h"

int is_prime2( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }

  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) {
      return 0;
    }
  }

  return 1;
}

void sleep (int t) {
  for (int i = 0; i < 5000 * t; i++) {
    int r = is_prime2( i ); 
  }
}

void main_P6() {
  while (1) {
    write( STDOUT_FILENO, "c", 1 );
    sleep(5);
    lcdColor();
    write( STDOUT_FILENO, "w", 1 );
    sleep(5);
    lcdWhite(); 
  }
  exit( EXIT_SUCCESS );
}
