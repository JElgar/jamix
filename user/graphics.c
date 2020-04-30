#include "graphics.h"
#include "fonts.h"

void put_pixel(int y, int x, uint16_t (*fb)[800], uint16_t c) {
  if (x < 800 && y < 600) {
    fb[y][x] = c;
  }
}

void square( uint32_t x, uint32_t y, uint32_t size, uint16_t color, uint16_t (*fb)[800] ) {
  if (size < 0) {
    return;
  }
  for (int i = 0; i < size; i++) { 
    if (i+y < 599) {
      for (int j = 0; j < size; j++) { 
        if (j+x < 799) {
          fb[i+y][j+x] = color;
        }
        else {
          break;
        }
      }
    }
    else {
      break;
    }
  }
}

void put_char(int c, uint32_t x, uint32_t y, uint16_t (*fb)[800]) {
  char *ci = font8x8_basic[c];
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (ci[i] & 1 << j) {
        put_pixel(y + i, x + j, fb, 0x0);
      }

    }
  }
}

void put_str(char* chars, uint32_t x, uint32_t y, uint16_t (*fb)[800]) {
  for (int i = 0; chars[i] != '\0'; i++) {
    put_char(chars[i], x + i * 10, y, fb);
  }
}

void button( buttonStruct button, char* text, uint16_t (*fb)[800] ) {
  square(button.x, button.y, button.size, button.color, fb);
  put_str(text, button.x, button.y + button.size + 10, fb);
}

void handleExecClick(int mouse_x, int mouse_y, int mouse_left_state, buttonStruct button, uint16_t (*fb)[800]) {
   // In the x range
   if (mouse_x > button.x && mouse_x < button.x + button.size ) {
    // If in the y range
    if (mouse_y > button.y && mouse_y < button.y + button.size ) {
      // If left mouse button pressed, execute prog
      if (mouse_left_state) exec(button.prog);
    }
   }
}

void handleTerminateClick(int mouse_x, int mouse_y, int mouse_left_state, int pid, buttonStruct button, uint16_t (*fb)[800]) {
   // In the x range
   if (mouse_x > button.x && mouse_x < button.x + button.size ) {
    // If in the y range
    if (mouse_y > button.y && mouse_y < button.y + button.size ) {
      // If left mouse button pressed, execute prog
      if (mouse_left_state) kill(pid, SIG_TERM);
    }
   }
}
