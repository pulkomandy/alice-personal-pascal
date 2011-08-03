/*
 * QNX system calls.
 * This is here because the ONYX preprocessor can't grok
 * #ifdef QNX
 * #asm ...
 * #endif
 * 
 * Now to the dirty deed of actually doing the system call.
 * Don't believe CEMCORP documentation.  To do a system call,
 * store the system call number obtained from "/lib/routines.h"
 * into callNumber, then push the system call arguments onto the
 * stack (i.e. do a regular C function call "systemCall(stdout, arg1, ...)".
 * 
 * systemCall stuffs the system call number into ax and does an interrupt,
 * which is fielded as a system call by the operating system.  It then
 * returns the result (if any) to the caller in the ax register.  Joy.
 */
#asm "seg 1"
#asm "<sys62>:"
#asm "mov ax,<callNumber>"
#asm "int 62h"
#asm "ret"
#asm "<sys72>:"
#asm "mov ax,<callNumber>"
#asm "int 72h"
#asm "ret"
#asm "seg 3"
