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
  bool clicked;
} buttonStruct;

// Draws a square at given x, y coords with given size and color 
extern void square( uint32_t x, uint32_t y, uint32_t size, uint16_t color, uint16_t (*fb)[800]);

// Puts character on screen using font from font.h at x, y coord
extern void put_char( int c, uint32_t x, uint32_t y, uint16_t color, uint16_t (*fb)[800]);

// Puts string on screen at x, y coord
extern void put_str(char* chars, uint32_t x, uint32_t y, uint16_t color, uint16_t (*fb)[800]);

// Puts give button on screen with helper text
extern void button( buttonStruct button, char* text, uint16_t textColor, uint16_t (*fb)[800]);

// Handles the hover and click iteractions of a given button for executing the buttons process
extern void handleExecClick(int mouse_x, int mouse_y, int mouse_left_state, buttonStruct *button, int *pid, uint16_t (*fb)[800]);

// Handles the hover and click iteractions of a given button for terminating the given process id
extern void handleTerminateClick(int mouse_x, int mouse_y, int mouse_left_state, int *pid, buttonStruct *button, uint16_t (*fb)[800]);

#endif
