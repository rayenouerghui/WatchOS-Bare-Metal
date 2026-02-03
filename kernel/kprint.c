#include "kprint.h"
#include "vga.h"

/* Initialize kernel printing system */
void kprint_init(void) {
    vga_init();
}

/* Print a kernel message with specified log level */
void kprint(log_level_t level, const char* message) {
    uint8_t color;
    const char* prefix;
    
    /* Determine color and prefix based on log level */
    switch (level) {
        case LOG_INFO:
            color = VGA_COLOR_WHITE;
            prefix = "[INFO] ";
            break;
        case LOG_OK:
            color = VGA_COLOR_LIGHT_GREEN;
            prefix = "[OK]   ";
            break;
        case LOG_WARN:
            color = VGA_COLOR_YELLOW;
            prefix = "[WARN] ";
            break;
        case LOG_ERROR:
            color = VGA_COLOR_LIGHT_RED;
            prefix = "[ERROR] ";
            break;
        default:
            color = VGA_COLOR_LIGHT_GRAY;
            prefix = "";
            break;
    }
    
    /* Print prefix and message */
    vga_print(prefix, color);
    vga_println(message, color);
}

/* Convenience wrappers for different log levels */
void kprint_info(const char* message) {
    kprint(LOG_INFO, message);
}

void kprint_ok(const char* message) {
    kprint(LOG_OK, message);
}

void kprint_warn(const char* message) {
    kprint(LOG_WARN, message);
}

void kprint_error(const char* message) {
    kprint(LOG_ERROR, message);
}

void kprint_hex(uint8_t value) {
    char hex[4] = "0x";
    const char* digits = "0123456789ABCDEF";
    hex[2] = digits[(value >> 4) & 0xF];
    hex[3] = digits[value & 0xF];
    
    vga_print(hex, VGA_COLOR_CYAN);
    vga_print(" ", VGA_COLOR_WHITE);
}
