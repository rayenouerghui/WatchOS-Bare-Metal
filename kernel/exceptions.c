#include "exceptions.h"
#include "vga.h"
#include "panic.h"

static const char* exception_messages[] = {
    "Divide by Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/* Convert number to hex string */
static void uint_to_hex(uint64_t value, char* buffer) {
    const char* digits = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (int i = 15; i >= 0; i--) {
        buffer[2 + (15 - i)] = digits[(value >> (i * 4)) & 0xF];
    }
    buffer[18] = '\0';
}

void exception_handler(uint64_t int_no, uint64_t err_code) {
    char hex_buffer[19];
    
    /* Clear screen and show error */
    vga_clear();
    vga_println("", VGA_COLOR_LIGHT_RED);
    vga_println("*** CPU EXCEPTION ***", VGA_COLOR_LIGHT_RED);
    vga_println("", VGA_COLOR_LIGHT_RED);
    
    /* Show exception name */
    vga_print("Exception: ", VGA_COLOR_WHITE);
    if (int_no < 32) {
        vga_println(exception_messages[int_no], VGA_COLOR_YELLOW);
    } else {
        vga_println("Unknown", VGA_COLOR_YELLOW);
    }
    
    /* Show exception number */
    vga_print("Number: ", VGA_COLOR_WHITE);
    uint_to_hex(int_no, hex_buffer);
    vga_println(hex_buffer, VGA_COLOR_CYAN);
    
    /* Show error code */
    vga_print("Error Code: ", VGA_COLOR_WHITE);
    uint_to_hex(err_code, hex_buffer);
    vga_println(hex_buffer, VGA_COLOR_CYAN);
    
    vga_println("", VGA_COLOR_WHITE);
    vga_println("System halted.", VGA_COLOR_LIGHT_GRAY);
    
    /* Halt the system */
    __asm__ volatile("cli");
    while (1) {
        __asm__ volatile("hlt");
    }
}
