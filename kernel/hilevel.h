/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <string.h>

// Include functionality relating to the   kernel.

#include     "int.h"

// Include timer, GIC (general input controller) and uart, (universal asyncrynous transmit recieve)
#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"

// LCD Imports
#include "PL050.h"
#include "PL111.h"
#include   "SYS.h"


#include "./queue/queue.h"
#include "../device/disk.h"

#define MAX_PROCS 27

typedef int pid_t;

typedef enum { 
  STATUS_INVALID,

  STATUS_CREATED,
  STATUS_TERMINATED,

  STATUS_READY,
  STATUS_EXECUTING,
  STATUS_WAITING
} status_t;

typedef struct {
  uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

typedef struct {
     pid_t    pid; // Process IDentifier (PID)
  status_t status; // current status
  uint32_t    tos; // address of Top of Stack (ToS)
     ctx_t    ctx; // execution context
} pcb_t; 

#include "./queue/priorityQueue.h"

// Buffer struct for pipes
typedef struct {
  bool inUse;      // Says if the buffer is inuse
  int length;      // Maximum size of buffer
  queue *data;      // Pointer to start of data array
} buffer;

void drawMousePointer();
#endif
