#include "graphics.h"

void square( uint32_t x, uint32_t y, uint32_t size, uint16_t (*fb)[800] ) {
  if (size < 0) {
    return;
  }
  for (int i = 0; i < size; i++) { 
    if (i+y < 599) {
      for (int j = 0; j < size; j++) { 
        if (j+x < 799) {
          fb[i+y][j+x] = 0x00;
        }
      }
    }
  }
}
