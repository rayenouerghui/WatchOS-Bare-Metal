#ifndef PANIC_H
#define PANIC_H

/* Kernel panic - called when a fatal error occurs
 * 
 * This function:
 * 1. Prints an error message to the screen
 * 2. Halts the CPU safely
 * 3. Never returns
 * 
 * Usage: panic("Out of memory!");
 */
void panic(const char* message) __attribute__((noreturn));

#endif
