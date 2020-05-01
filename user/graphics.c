#include "graphics.h"
#include "fonts.h"

// Helper function to keep in range of frambuffer
void put_pixel(int y, int x, uint16_t (*fb)[800], uint16_t c) {
  if (x < 800 && y < 600) {
    fb[y][x] = c;
  } 
}

void square( uint32_t x, uint32_t y, uint32_t size, uint16_t color, uint16_t (*fb)[800] ) {
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

void put_char(int c, uint32_t x, uint32_t y, uint16_t color, uint16_t (*fb)[800]) {
  // Find character row
  char *ci = font8x8_basic[c];
  // Draw pixels at points specified in font array
  // Fonts are stored in array as 8x8 grid
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (ci[i] & 1 << j) {
        put_pixel(y + i, x + j, fb, color);
      }
    }
  }
}

void put_str(char* chars, uint32_t x, uint32_t y, uint16_t color, uint16_t (*fb)[800]) {
  // Loop thorugh char* until hit string terminating character and put_char
  for (int i = 0; chars[i] != '\0'; i++) {
    put_char(chars[i], x + i * 10, y, color, fb);
  }
}

void button( buttonStruct button, char* text, uint16_t textColor, uint16_t (*fb)[800] ) {
  // Draw square at button location
  square(button.x, button.y, button.size, button.color, fb);
  // Write helper text under button
  put_str(text, button.x, button.y + button.size + 10, button.color, fb);
}

void handleExecClick(int mouse_x, int mouse_y, int mouse_left_state, buttonStruct *button,int *pid, uint16_t (*fb)[800]) {
   // In the x and y range of the button
   if ((mouse_x > button->x && mouse_x < button->x + button->size) && (mouse_y > button->y && mouse_y < button->y + button->size )) {
      if (!button->hovering) {
        // Sets hovering as true in hilevel so mouse can change
        setHover(true);
        // Set hovering in button so knows its already been done
        // This prevents having to redraw every frame
        button->hovering = true;
        // Draw square over current button to change color
        square(button->x, button->y, button->size, 0x0FF, fb);
        draw();
      }
      // If left mouse button pressed, execute prog
      if (mouse_left_state && !button->clicked) {
        // Revert to unhovered once clicked
        button->clicked = true;
        setHover(false);
        button->hovering = false;
        square(button->x, button->y, button->size, button->color, fb);
        int f = fork();
        if( 0 == f ) {
          exec( button->prog );
        } else {
          put_char((*pid + '0'), 680, 410, 0x7FFF, fb);
          *pid = f;
        }
        draw();
      }
   }
   else {
     if (button->hovering) {
       // Revert to unhovered once left button area
       setHover(false);
       button->hovering = false;
       square(button->x, button->y, button->size, button->color, fb);
       button->clicked = false;
       draw();
     }
   }
}

void handleTerminateClick(int mouse_x, int mouse_y, int mouse_left_state, int *pid, buttonStruct *button, uint16_t (*fb)[800]) {
   // In the x and y range
   if ((mouse_x > button->x && mouse_x < button->x + button->size) && (mouse_y > button->y && mouse_y < button->y + button->size )) {
      // If left mouse button pressed, terminate pid
      if (!button->hovering) {
        // Sets hovering as true in hilevel so mouse can change
        setHover(true);
        // Set hovering in button so knows its already been done
        // This prevents having to redraw every frame
        button->hovering = true;
        // Draw square over current button to change color
        square(button->x, button->y, button->size, 0x2FA, fb);
        draw();
      }
      if (mouse_left_state ) {
        kill(*pid, SIG_TERM);
        put_char((*pid + '0'), 680, 410, 0x7FFF, fb);
        square(680, 410, 8, 0x7C00, fb);
        draw();
      }
   }
   else {
     if (button->hovering) {
       // Revert to unhovered once left button area
       setHover(false);
       button->hovering = false;
       square(button->x, button->y, button->size, button->color, fb);
       button->clicked = false;
       draw();
     }
   }
}
