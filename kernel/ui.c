#include "ui.h"
#include "vga.h"
#include "timer.h"
#include "pmm.h"
#include "heap.h"

#define NUM_BUTTONS 3

static button_t buttons[NUM_BUTTONS];
static uint8_t current_selection = 0;
static uint8_t in_menu = 1;

/* Blocky ASCII art logo like Claude style */
static const char* logo[] = {
    " ####    ###   #   # ##### #   #       ####   #### ",
    " #   #  #   #  #   # #      # #       #    # #     ",
    " ####   #####   # #  ####    #        #    #  ###  ",
    " #  #   #   #    #   #       #        #    #     # ",
    " #   #  #   #    #   #####   #         ####  ####  "
};

void ui_init(void) {
    /* Initialize buttons - vertical centered layout */
    buttons[0].x = 28;
    buttons[0].y = 12;
    buttons[0].width = 24;
    buttons[0].height = 3;
    buttons[0].label = "   [1] Show Time    ";
    buttons[0].selected = 1;
    
    buttons[1].x = 28;
    buttons[1].y = 16;
    buttons[1].width = 24;
    buttons[1].height = 3;
    buttons[1].label = "   [2] Snake Game   ";
    buttons[1].selected = 0;
    
    buttons[2].x = 28;
    buttons[2].y = 20;
    buttons[2].width = 24;
    buttons[2].height = 3;
    buttons[2].label = "  [3] System Info   ";
    buttons[2].selected = 0;
    
    current_selection = 0;
    in_menu = 1;
}

static void draw_logo(void) {
    uint8_t start_y = 2;
    uint8_t start_x = 14;
    
    for (int i = 0; i < 5; i++) {
        vga_set_cursor(start_x, start_y + i);
        vga_print(logo[i], VGA_COLOR_LIGHT_CYAN);
    }
}

static void draw_button(button_t* btn) {
    uint8_t color = btn->selected ? VGA_COLOR_YELLOW : VGA_COLOR_WHITE;
    uint8_t border_color = btn->selected ? VGA_COLOR_YELLOW : VGA_COLOR_LIGHT_GRAY;
    
    /* Top border */
    vga_set_cursor(btn->x, btn->y);
    vga_print("+", border_color);
    for (int i = 0; i < btn->width - 2; i++) {
        vga_print("-", border_color);
    }
    vga_print("+", border_color);
    
    /* Label */
    vga_set_cursor(btn->x, btn->y + 1);
    vga_print("|", border_color);
    vga_print(btn->label, color);
    vga_print("|", border_color);
    
    /* Bottom border */
    vga_set_cursor(btn->x, btn->y + 2);
    vga_print("+", border_color);
    for (int i = 0; i < btn->width - 2; i++) {
        vga_print("-", border_color);
    }
    vga_print("+", border_color);
}

void ui_draw_menu(void) {
    vga_clear();
    
    /* Draw logo */
    draw_logo();
    
    /* Draw subtitle */
    vga_set_cursor(15, 8);
    vga_print("Educational Operating System - Built from Scratch", VGA_COLOR_LIGHT_GRAY);
    
    /* Draw buttons */
    for (int i = 0; i < NUM_BUTTONS; i++) {
        draw_button(&buttons[i]);
    }
    
    /* Draw instructions at bottom */
    vga_set_cursor(22, 24);
    vga_print("Press [1] [2] [3] to select  |  [ESC] to exit", VGA_COLOR_DARK_GRAY);
}

void ui_handle_input(uint8_t scancode) {
    if (!in_menu) return;
    
    /* Number keys 1, 2, 3 */
    if (scancode >= 0x02 && scancode <= 0x04) {
        uint8_t choice = scancode - 0x02;
        
        /* Update selection */
        buttons[current_selection].selected = 0;
        current_selection = choice;
        buttons[current_selection].selected = 1;
        
        /* Redraw menu */
        ui_draw_menu();
        
        /* Small delay */
        for (volatile int i = 0; i < 10000000; i++);
        
        /* Execute */
        in_menu = 0;
        switch (choice) {
            case 0:
                ui_show_time();
                break;
            case 1:
                ui_show_snake();
                break;
            case 2:
                ui_show_sysinfo();
                break;
        }
        in_menu = 1;
        ui_draw_menu();
    }
}

void ui_show_time(void) {
    vga_clear();
    
    vga_set_cursor(30, 5);
    vga_print("=== SYSTEM TIME ===", VGA_COLOR_LIGHT_CYAN);
    
    uint64_t ticks = timer_get_ticks();
    uint64_t seconds = ticks / 100;
    uint64_t minutes = seconds / 60;
    uint64_t hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    
    vga_set_cursor(25, 10);
    vga_print("System Uptime:", VGA_COLOR_WHITE);
    
    vga_set_cursor(30, 12);
    vga_print("Hours:   ", VGA_COLOR_LIGHT_GRAY);
    print_number(hours);
    
    vga_set_cursor(30, 13);
    vga_print("Minutes: ", VGA_COLOR_LIGHT_GRAY);
    print_number(minutes);
    
    vga_set_cursor(30, 14);
    vga_print("Seconds: ", VGA_COLOR_LIGHT_GRAY);
    print_number(seconds);
    
    vga_set_cursor(30, 16);
    vga_print("Total Ticks: ", VGA_COLOR_LIGHT_GRAY);
    print_number(ticks);
    
    vga_set_cursor(25, 20);
    vga_print("Press ESC to return to menu...", VGA_COLOR_YELLOW);
    
    while (1) {
        __asm__ volatile("hlt");
    }
}

void ui_show_snake(void) {
    vga_clear();
    
    vga_set_cursor(32, 2);
    vga_print("=== SNAKE GAME ===", VGA_COLOR_LIGHT_GREEN);
    
    /* Game border */
    for (int x = 10; x < 70; x++) {
        vga_set_cursor(x, 5);
        vga_print("#", VGA_COLOR_LIGHT_GRAY);
        vga_set_cursor(x, 20);
        vga_print("#", VGA_COLOR_LIGHT_GRAY);
    }
    for (int y = 5; y <= 20; y++) {
        vga_set_cursor(10, y);
        vga_print("#", VGA_COLOR_LIGHT_GRAY);
        vga_set_cursor(69, y);
        vga_print("#", VGA_COLOR_LIGHT_GRAY);
    }
    
    /* Snake */
    vga_set_cursor(40, 12);
    vga_print("@", VGA_COLOR_LIGHT_GREEN);
    
    /* Food */
    vga_set_cursor(50, 15);
    vga_print("*", VGA_COLOR_LIGHT_RED);
    
    vga_set_cursor(15, 22);
    vga_print("Use WASD to move  |  ESC to return", VGA_COLOR_YELLOW);
    
    vga_set_cursor(20, 23);
    vga_print("(Demo version - full game coming soon!)", VGA_COLOR_DARK_GRAY);
    
    while (1) {
        __asm__ volatile("hlt");
    }
}

void ui_show_sysinfo(void) {
    vga_clear();
    
    vga_set_cursor(28, 3);
    vga_print("=== SYSTEM INFORMATION ===", VGA_COLOR_LIGHT_CYAN);
    
    vga_set_cursor(15, 6);
    vga_print("Operating System:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 6);
    vga_print("RayenOS v1.0", VGA_COLOR_LIGHT_GREEN);
    
    vga_set_cursor(15, 7);
    vga_print("Architecture:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 7);
    vga_print("x86_64", VGA_COLOR_LIGHT_GREEN);
    
    vga_set_cursor(15, 8);
    vga_print("Kernel:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 8);
    vga_print("Freestanding", VGA_COLOR_LIGHT_GREEN);
    
    vga_set_cursor(15, 10);
    vga_print("=== Memory Statistics ===", VGA_COLOR_YELLOW);
    
    uint64_t total = pmm_get_total_memory();
    uint64_t used = pmm_get_used_memory();
    uint64_t free = pmm_get_free_memory();
    
    vga_set_cursor(15, 12);
    vga_print("Total Memory:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 12);
    print_number(total / 1024);
    vga_print(" KB", VGA_COLOR_LIGHT_GRAY);
    
    vga_set_cursor(15, 13);
    vga_print("Used Memory:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 13);
    print_number(used / 1024);
    vga_print(" KB", VGA_COLOR_LIGHT_GRAY);
    
    vga_set_cursor(15, 14);
    vga_print("Free Memory:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 14);
    print_number(free / 1024);
    vga_print(" KB", VGA_COLOR_LIGHT_GRAY);
    
    uint64_t heap_total, heap_used, heap_free;
    heap_stats(&heap_total, &heap_used, &heap_free);
    
    vga_set_cursor(15, 16);
    vga_print("Heap Total:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 16);
    print_number(heap_total / 1024);
    vga_print(" KB", VGA_COLOR_LIGHT_GRAY);
    
    uint64_t ticks = timer_get_ticks();
    uint64_t seconds = ticks / 100;
    
    vga_set_cursor(15, 18);
    vga_print("System Uptime:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 18);
    print_number(seconds);
    vga_print(" seconds", VGA_COLOR_LIGHT_GRAY);
    
    vga_set_cursor(25, 22);
    vga_print("Press ESC to return to menu...", VGA_COLOR_YELLOW);
    
    while (1) {
        __asm__ volatile("hlt");
    }
}

void print_number(uint64_t num) {
    if (num == 0) {
        vga_print("0", VGA_COLOR_LIGHT_CYAN);
        return;
    }
    
    char buffer[20];
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    for (int j = i - 1; j >= 0; j--) {
        char c[2] = {buffer[j], '\0'};
        vga_print(c, VGA_COLOR_LIGHT_CYAN);
    }
}
