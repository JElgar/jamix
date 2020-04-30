#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t size;
  uint16_t color;
  void* prog; 
  bool hovering;
} buttonStruct;

extern void square( uint32_t x, uint32_t y, uint32_t size, uint16_t color, uint16_t (*fb)[800]);
extern void put_char( int c, uint32_t x, uint32_t y, uint16_t (*fb)[800]);
extern void button( buttonStruct button, char* text, uint16_t (*fb)[800]);
extern void handleExecClick(int mouse_x, int mouse_y, int mouse_left_state, buttonStruct *button, uint16_t (*fb)[800]);
extern void handleTerminateClick(int mouse_x, int mouse_y, int mouse_left_state, int pid, buttonStruct *button, uint16_t (*fb)[800]);

#endif
