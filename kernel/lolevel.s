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

                     pop { r0, lr }                @ load     USR mode PC and CPSR
                     msr   spsr, r0                @ move     USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ restore  USR mode registers
                     add   sp, sp, #60             @ update   SVC mode SP
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_irq: sub   lr, lr, #4              @ correct return address
                    
                     /* hilevel handler reqires a context, the context is the current state of any usr (as all programs run in usermode, registers)
                      * therefore push all these values onto stack and give the stack pointer as the pointer to the values on the stack
                      */
                     sub   sp, sp, #60             @ update   SVC mode stack
                     stmia sp, { r0-r12, sp, lr }^ @ preserve USR registers
                     mrs   r0, spsr                @ move     USR        CPSR
                     /*push { r0, lr }               @ store    USR PC and CPSR*/
                     stmdb sp!, {r0, lr}
         
                     /* save    caller-save registers 
                      * ^ means user mode registers
                      *  cannot add register pc here because the order of register pushed onto stack is imporntatn and would be in "register order" otherwise
                      */

                     /* cannot push status register onto stack, instead push spsr which stores the previous *usr* mode value
                      * cannot just push spsr (cause its a status register
                      * therefore use mrs, which copies status register value, then push that register
                      */
                     /* Not sure why db used */

                     mov r0, sp                     

                     bl    hilevel_handler_irq     @ invoke high-level C function
                     
                     ldmia sp!, { r0, lr}
                     /*pop { r0, lr }                @ load     USR mode PC and CPSR*/
                     msr   spsr, r0                @ move     USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ restore  USR mode registers
                     add   sp, sp, #60             @ update   SVC mode SP
                     movs  pc, lr                  @ return from interrupt

                     /* clean up stack and restore context. The final lines adds values of the new process, the rest remove the values and move the stack pointer to its previous location
                      */

/*
lolevel_handler_svc: sub   lr, lr, #0              @ correct return address
                     stmfd sp, { r0-r12, lr, sp }^  @ save    caller-save registers
                     
                     mrs r0, spsr
                     stmdb sp, {r0, pc}
                     mov r0, sp                     

                     bl    hilevel_handler_svc     @ invoke high-level C function
                     
                     ldmia sp, {r0, pc}
                     ldmfd sp, { r0-r12, lr, sp }^  @ restore caller-save registers
                     movs  pc, lr                  @ return from interrupt 
*/

lolevel_handler_svc:          
                     sub   lr, lr, #0              @ update   SVC mode stack
                     sub   sp, sp, #60             @ update   SVC mode stack
                     stmia sp, { r0-r12, sp, lr }^ @ preserve USR registers
                     mrs   r0, spsr                @ move     USR        CPSR
                     stmdb sp!, {r0, lr}
                     /*push { r0, lr }               @ store    USR PC and CPSR*/
         
                     mov   r0, sp                  @ set    high-level C function arg. = SP
                     /* Gets operand of svc call to get value of svc call */
                     ldr   r1, [ lr, #-4 ]         @ load                     svc instruction
                     bic   r1, r1, #0xFF000000     @ set    high-level C function arg. = svc immediate
                     bl    hilevel_handler_svc     @ invoke high-level C function
        
                     /*pop { r0, lr }                @ load     USR mode PC and CPSR*/
                     ldmia sp!, { r0, lr}
                     msr   spsr, r0                @ move     USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ restore  USR mode registers
                     add   sp, sp, #60             @ update   SVC mode SP
                     movs  pc, lr                  @ return from interrupt
