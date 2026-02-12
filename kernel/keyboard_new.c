#include <stdint.h>
#include "ui.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    /* Handle key press only (bit 7 is 0) */
    if (!(scancode & 0x80)) {
        /* Force exit on ESC or X */
        if (scancode == 0x01 || scancode == 0x2D) {
            ui_request_exit();
        }
        /* Pass to UI handler (menu + sub-screens) */
        ui_handle_input(scancode);
    }
}
