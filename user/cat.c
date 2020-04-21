#include "cat.h"

void main_cat( uint32_t address, int size ) {
  uint8_t* val;
  disk_rd(address, val, size);
}
