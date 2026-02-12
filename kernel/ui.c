#include "ui.h"
#include "vga.h"
#include "timer.h"
#include "pmm.h"
#include "heap.h"

#define NUM_BUTTONS 3

static button_t buttons[NUM_BUTTONS];
static uint8_t current_selection = 0;
static uint8_t in_menu = 1;

/* ASCII Art for "Rayen OS" */
static const char* logo[] = {
    "  ____                          ___  ____  ",
    " |  _ \\ __ _ _   _  ___ _ __   / _ \\/ ___| ",
    " | |_) / _` | | | |/ _ \\ '_ \\ | | | \\___ \\ ",
    " |  _ < (_| | |_| |  __/ | | || |_| |___) |",
    " |_| \\_\\__,_|\\__, |\\___|_| |_| \\___/|____/ ",
    "             |___/                          "
};

void ui_init(void) {
    /* Initialize buttons */
    buttons[0].x = 15;
    buttons[0].y = 15;
    buttons[0].width = 20;
    buttons[0].height = 3;
    buttons[0].label = "  [1] Show Time  ";
    buttons[0].selected = 1;
    
    buttons[1].x = 15;
    buttons[1].y = 19;
    buttons[1].width = 20;
    buttons[1].height = 3;
    buttons[1].label = "  [2] Snake Game ";
    buttons[1].selected = 0;
    
    buttons[2].x = 45;
    buttons[2].y = 15;
    buttons[2].width = 20;
    buttons[2].height = 3;
    buttons[2].label = " [3] System Info";
    buttons[2].selected = 0;
    
    current_selection = 0;
    in_menu = 1;
}

static void draw_logo(void) {
    uint8_t start_y = 3;
    uint8_t start_x = 17;
    
    for (int i = 0; i < 6; i++) {
        vga_set_cursor(start_x, start_y + i);
        vga_print(logo[i], VGA_COLOR_LIGHT_CYAN);
    }
}

static void draw_button(button_t* btn) {
    uint8_t color = btn->selected ? VGA_COLOR_YELLOW : VGA_COLOR_WHITE;
    uint8_t bg_color = btn->selected ? VGA_COLOR_BLUE : VGA_COLOR_BLACK;
    
    /* Draw button border */
    vga_set_cursor(btn->x, btn->y);
    vga_print("+", color);
    for (int i = 0; i < btn->width - 2; i++) {
        vga_print("-", color);
    }
    vga_print("+", color);
    
    /* Draw button label */
    vga_set_cursor(btn->x, btn->y + 1);
    vga_print("|", color);
    vga_print(btn->label, btn->selected ? VGA_COLOR_BLACK : color);
    vga_print("|", color);
    
    /* Draw bottom border */
    vga_set_cursor(btn->x, btn->y + 2);
    vga_print("+", color);
    for (int i = 0; i < btn->width - 2; i++) {
        vga_print("-", color);
    }
    vga_print("+", color);
}

void ui_draw_menu(void) {
    vga_clear();
    
    /* Draw logo */
    draw_logo();
    
    /* Draw subtitle */
    vga_set_cursor(25, 10);
    vga_print("Welcome to RayenOS - Educational OS", VGA_COLOR_LIGHT_GRAY);
    
    /* Draw buttons */
    for (int i = 0; i < NUM_BUTTONS; i++) {
        draw_button(&buttons[i]);
    }
    
    /* Draw instructions */
    vga_set_cursor(20, 23);
    vga_print("Use [1] [2] [3] to select  |  [ESC] to return", VGA_COLOR_DARK_GRAY);
}

void ui_handle_input(uint8_t scancode) {
    if (!in_menu) return;
    
    /* Number keys 1, 2, 3 */
    if (scancode >= 0x02 && scancode <= 0x04) {
        uint8_t choice = scancode - 0x02;  /* 0, 1, 2 */
        
        /* Update selection */
        buttons[current_selection].selected = 0;
        current_selection = choice;
        buttons[current_selection].selected = 1;
        
        /* Redraw menu */
        ui_draw_menu();
        
        /* Small delay then execute */
        for (volatile int i = 0; i < 10000000; i++);
        
        /* Execute selected action */
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
    
    /* Title */
    vga_set_cursor(30, 5);
    vga_print("=== SYSTEM TIME ===", VGA_COLOR_LIGHT_CYAN);
    
    /* Get ticks */
    uint64_t ticks = timer_get_ticks();
    uint64_t seconds = ticks / 100;  /* 100 Hz timer */
    uint64_t minutes = seconds / 60;
    uint64_t hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    
    /* Display uptime */
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
    
    /* Instructions */
    vga_set_cursor(25, 20);
    vga_print("Press ESC to return to menu...", VGA_COLOR_YELLOW);
    
    /* Wait for ESC */
    while (1) {
        __asm__ volatile("hlt");
        /* ESC key will be handled by keyboard driver */
    }
}

void ui_show_snake(void) {
    vga_clear();
    
    /* Title */
    vga_set_cursor(32, 2);
    vga_print("=== SNAKE GAME ===", VGA_COLOR_LIGHT_GREEN);
    
    /* Game area border */
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
    
    /* Snake initial position */
    uint8_t snake_x = 40;
    uint8_t snake_y = 12;
    
    vga_set_cursor(snake_x, snake_y);
    vga_print("@", VGA_COLOR_LIGHT_GREEN);
    
    /* Food */
    vga_set_cursor(50, 15);
    vga_print("*", VGA_COLOR_LIGHT_RED);
    
    /* Instructions */
    vga_set_cursor(15, 22);
    vga_print("Use WASD to move  |  ESC to return", VGA_COLOR_YELLOW);
    
    vga_set_cursor(25, 23);
    vga_print("(Simple demo - full game coming soon!)", VGA_COLOR_DARK_GRAY);
    
    /* Wait for ESC */
    while (1) {
        __asm__ volatile("hlt");
    }
}

void ui_show_sysinfo(void) {
    vga_clear();
    
    /* Title */
    vga_set_cursor(28, 3);
    vga_print("=== SYSTEM INFORMATION ===", VGA_COLOR_LIGHT_CYAN);
    
    /* OS Info */
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
    vga_print("Freestanding (No libc)", VGA_COLOR_LIGHT_GREEN);
    
    /* Memory Info */
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
    
    /* Heap Info */
    uint64_t heap_total, heap_used, heap_free;
    heap_stats(&heap_total, &heap_used, &heap_free);
    
    vga_set_cursor(15, 16);
    vga_print("Heap Total:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 16);
    print_number(heap_total / 1024);
    vga_print(" KB", VGA_COLOR_LIGHT_GRAY);
    
    vga_set_cursor(15, 17);
    vga_print("Heap Used:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 17);
    print_number(heap_used / 1024);
    vga_print(" KB", VGA_COLOR_LIGHT_GRAY);
    
    /* Uptime */
    uint64_t ticks = timer_get_ticks();
    uint64_t seconds = ticks / 100;
    
    vga_set_cursor(15, 19);
    vga_print("System Uptime:", VGA_COLOR_WHITE);
    vga_set_cursor(35, 19);
    print_number(seconds);
    vga_print(" seconds", VGA_COLOR_LIGHT_GRAY);
    
    /* Instructions */
    vga_set_cursor(25, 22);
    vga_print("Press ESC to return to menu...", VGA_COLOR_YELLOW);
    
    /* Wait for ESC */
    while (1) {
        __asm__ volatile("hlt");
    }
}

/* Helper function to print numbers */
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
    
    /* Reverse and print */
    for (int j = i - 1; j >= 0; j--) {
        char c[2] = {buffer[j], '\0'};
        vga_print(c, VGA_COLOR_LIGHT_CYAN);
    }
}
