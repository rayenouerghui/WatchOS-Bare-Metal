#include "vga.h"

/* VGA text buffer address */
static volatile uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;

/* Current cursor position */
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

/* Helper: create VGA entry (character + color) */
static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

/* Helper: get buffer index from x,y coordinates */
static inline uint16_t vga_index(uint8_t x, uint8_t y) {
    return y * VGA_WIDTH + x;
}

/* Initialize VGA driver */
void vga_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    vga_clear();
}

/* Clear the entire screen */
void vga_clear(void) {
    for (uint32_t i = 0; i < (uint32_t)(VGA_WIDTH * VGA_HEIGHT); i++) {
        VGA_BUFFER[i] = vga_entry(' ', VGA_COLOR_LIGHT_GRAY);
    }
    cursor_x = 0;
    cursor_y = 0;
}

/* Scroll screen up by one line */
static void vga_scroll(void) {
    /* Move all lines up by one */
    for (uint8_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (uint8_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[vga_index(x, y)] = VGA_BUFFER[vga_index(x, y + 1)];
        }
    }
    
    /* Clear the last line */
    for (uint8_t x = 0; x < VGA_WIDTH; x++) {
        VGA_BUFFER[vga_index(x, VGA_HEIGHT - 1)] = vga_entry(' ', VGA_COLOR_LIGHT_GRAY);
    }
    
    cursor_y = VGA_HEIGHT - 1;
}

/* Move cursor to next line */
static void vga_newline(void) {
    cursor_x = 0;
    cursor_y++;
    
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
    }
}

/* Print a single character at current cursor position */
void vga_putchar(char c, uint8_t color) {
    if (c == '\n') {
        vga_newline();
        return;
    }
    
    if (c == '\r') {
        cursor_x = 0;
        return;
    }
    
    /* Write character to buffer */
    VGA_BUFFER[vga_index(cursor_x, cursor_y)] = vga_entry(c, color);
    
    /* Advance cursor */
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        vga_newline();
    }
}

/* Print a null-terminated string */
void vga_print(const char* str, uint8_t color) {
    while (*str) {
        vga_putchar(*str, color);
        str++;
    }
}

/* Print a string and move to next line */
void vga_println(const char* str, uint8_t color) {
    vga_print(str, color);
    vga_newline();
}

/* Set cursor position manually */
void vga_set_cursor(uint8_t x, uint8_t y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
    }
}
