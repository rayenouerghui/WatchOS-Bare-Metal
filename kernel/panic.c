#include "panic.h"
#include "vga.h"

/* Halt the CPU safely in an infinite loop */
static inline void halt(void) {
    while (1) {
        __asm__ volatile("cli");  /* Disable interrupts */
        __asm__ volatile("hlt");  /* Halt CPU */
    }
}

/* Kernel panic - fatal error handler
 * 
 * This function is called when the kernel encounters an unrecoverable error.
 * It displays a clear error message and stops the system safely.
 * 
 * The __attribute__((noreturn)) tells the compiler this function never returns,
 * which helps with optimization and prevents warnings.
 */
void panic(const char* message) {
    /* Clear screen and display panic message */
    vga_clear();
    
    /* Print panic header in red */
    vga_println("", VGA_COLOR_LIGHT_RED);
    vga_println("*** KERNEL PANIC ***", VGA_COLOR_LIGHT_RED);
    vga_println("", VGA_COLOR_LIGHT_RED);
    
    /* Print the error message */
    vga_print("Fatal error: ", VGA_COLOR_WHITE);
    vga_println(message, VGA_COLOR_YELLOW);
    
    vga_println("", VGA_COLOR_WHITE);
    vga_println("System halted. Please reboot.", VGA_COLOR_LIGHT_GRAY);
    
    /* Halt the CPU - never returns */
    halt();
    
    /* Should never reach here, but satisfy compiler */
    __builtin_unreachable();
}
