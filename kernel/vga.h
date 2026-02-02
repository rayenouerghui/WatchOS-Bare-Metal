#ifndef VGA_H
#define VGA_H

#include <stdint.h>

/* VGA color codes */
#define VGA_COLOR_BLACK         0x0
#define VGA_COLOR_BLUE          0x1
#define VGA_COLOR_GREEN         0x2
#define VGA_COLOR_CYAN          0x3
#define VGA_COLOR_RED           0x4
#define VGA_COLOR_MAGENTA       0x5
#define VGA_COLOR_BROWN         0x6
#define VGA_COLOR_LIGHT_GRAY    0x7
#define VGA_COLOR_DARK_GRAY     0x8
#define VGA_COLOR_LIGHT_BLUE    0x9
#define VGA_COLOR_LIGHT_GREEN   0xA
#define VGA_COLOR_LIGHT_CYAN    0xB
#define VGA_COLOR_LIGHT_RED     0xC
#define VGA_COLOR_LIGHT_MAGENTA 0xD
#define VGA_COLOR_YELLOW        0xE
#define VGA_COLOR_WHITE         0xF

/* VGA dimensions */
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

/* Initialize VGA driver */
void vga_init(void);

/* Clear the screen */
void vga_clear(void);

/* Print a single character at current cursor position */
void vga_putchar(char c, uint8_t color);

/* Print a string with specified color */
void vga_print(const char* str, uint8_t color);

/* Print a string and move to next line */
void vga_println(const char* str, uint8_t color);

/* Set cursor position */
void vga_set_cursor(uint8_t x, uint8_t y);

#endif
