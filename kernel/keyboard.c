#include <stdint.h>
#include "kprint.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* US QWERTY scancode to ASCII mapping (set 1) */
static const char scancode_to_ascii[] = {
    0,   27,  '1', '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

/* Shifted characters */
static const char scancode_to_ascii_shifted[] = {
    0,   27,  '!', '@', '#', '$',$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

static uint8_t shift_pressed = 0;

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    /* Handle key press (bit 7 is 0) */
    if (!(scancode & 0x80)) {
        /* Left shift (0x2A) or right shift (0x36) pressed */
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            return;
        }
        
        /* Convert scancode to ASCII */
        if (scancode < sizeof(scancode_to_ascii)) {
            char c;
            if (shift_pressed) {
                c = scancode_to_ascii_shifted[scancode];
            } else {
                c = scancode_to_ascii[scancode];
            }
            
            if (c != 0) {
                /* Print the character */
                vga_putchar(c, VGA_COLOR_WHITE);
            }
        }
    } else {
        /* Key release (bit 7 is 1) */
        scancode &= 0x7F;  // Remove release bit
        
        /* Left shift or right shift released */
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 0;
        }
    }
}
