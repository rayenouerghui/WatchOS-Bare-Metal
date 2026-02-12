#ifndef UI_H
#define UI_H

#include <stdint.h>

/* UI button structure */
typedef struct {
    uint8_t x, y;           /* Position */
    uint8_t width, height;  /* Size */
    const char* label;      /* Button text */
    uint8_t selected;       /* Is selected */
} button_t;

/* Initialize UI system */
void ui_init(void);

/* Draw the main menu */
void ui_draw_menu(void);

/* Handle keyboard input for menu */
void ui_handle_input(uint8_t scancode);

/* Show time screen */
void ui_show_time(void);

/* Show snake game */
void ui_show_snake(void);

/* Show system info */
void ui_show_sysinfo(void);

/* Helper function to print numbers */
void print_number(uint64_t num);

#endif
