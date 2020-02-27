/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

/* Each of the following is a low-level interrupt handler: each one is
 * tasked with handling a different interrupt type, and acts as a sort
 * of wrapper around a high-level, C-based handler.
 */

.global lolevel_handler_rst
.global lolevel_handler_irq
.global lolevel_handler_svc

lolevel_handler_rst: bl    int_init                @ initialise interrupt vector table

                     msr   cpsr, #0xD2             @ enter IRQ mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_irq            @ initialise IRQ mode stack
                     msr   cpsr, #0xD3             @ enter SVC mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_svc            @ initialise SVC mode stack
                      
                     sub sp, sp, #68               @ Remove 68 from the stack pointer because reasons
                     mov r0, sp                    @ Store stack pointer in r0 to be parsed into function as context

                     bl    hilevel_handler_rst     @ invoke high-level C function
                     b     .                       @ halt

lolevel_handler_irq: sub   lr, lr, #4              @ correct return address
                    
                     /* hilevel handler reqires a context, the context is the current state of any usr (as all programs run in usermode, registers)
                      * therefore push all these values onto stack and give the stack pointer as the pointer to the values on the stack
                      */
                     sub sp, sp, #60               @ preset stack pointer so return to same posiiton after increment after
                     /* save    caller-save registers 
                      * ^ means user mode registers
                      *  cannot add register pc here because the order of register pushed onto stack is imporntatn and would be in "register order" otherwise
                      */
                     stmia sp, { r0-r12, lr, sp }^  @ 

                     /* cannot push status register onto stack, instead push spsr which stores the previous *usr* mode value
                      * cannot just push spsr (cause its a status register
                      * therefore use mrs, which copies status register value, then push that register
                      */
                     mrs r0, spsr
                     push {pc, r0}

                     mov r0, sp                     

                     bl    hilevel_handler_irq     @ invoke high-level C function

                     /* clean up stack and restore context. The final lines adds values of the new process, the rest remove the values and move the stack pointer to its previous location
                      */
                     ldmfd sp!, { r0-r12, lr, sp }     @ restore caller-save registers
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_svc: sub   lr, lr, #0              @ correct return address
                     stmfd sp!, { r0-r3, ip, lr }  @ save    caller-save registers

                     bl    hilevel_handler_svc     @ invoke high-level C function

                     ldmfd sp!, { r0-r3, ip, lr }  @ restore caller-save registers
                     movs  pc, lr                  @ return from interrupt 
