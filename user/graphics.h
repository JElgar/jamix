#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"

extern void square( uint32_t x, uint32_t y, uint32_t size, uint16_t (*fb)[800]);

#endif
