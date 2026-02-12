#include "ui.h"
#include "vga.h"
#include "timer.h"
#include "pmm.h"
#include "heap.h"

#define NUM_BUTTONS 3

static button_t buttons[NUM_BUTTONS];
static uint8_t current_selection = 0;
typedef enum {
    UI_SCREEN_MENU = 0,
    UI_SCREEN_TIME,
    UI_SCREEN_SNAKE,
    UI_SCREEN_SYSINFO
} ui_screen_t;

static volatile ui_screen_t current_screen = UI_SCREEN_MENU;
static volatile uint8_t screen_dirty = 1;
static volatile uint8_t ui_exit_requested = 0;
static volatile uint8_t ui_last_scancode = 0;

/* Snake state */
static uint8_t snake_x = 40;
static uint8_t snake_y = 12;
static uint8_t snake_dir = 1;
static uint64_t snake_last_tick = 0;
static uint64_t snake_tick_interval = 20;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void ui_request_exit(void) {
    ui_exit_requested = 1;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static uint8_t cmos_read(uint8_t reg) {
    outb(0x70, (uint8_t)(reg | 0x80));
    return inb(0x71);
}

static uint8_t bcd_to_bin(uint8_t v) {
    return (uint8_t)((v & 0x0F) + ((v >> 4) * 10));
}

static void rtc_read_time(uint8_t* hours, uint8_t* minutes, uint8_t* seconds) {
    /* Wait until RTC is not updating */
    while (cmos_read(0x0A) & 0x80) {
    }

    uint8_t sec = cmos_read(0x00);
    uint8_t min = cmos_read(0x02);
    uint8_t hr  = cmos_read(0x04);
    uint8_t regb = cmos_read(0x0B);

    uint8_t is_bcd = ((regb & 0x04) == 0);
    uint8_t is_24h = ((regb & 0x02) != 0);

    if (is_bcd) {
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        /* In 12h mode, hour has PM bit in bit 7 */
        if (!is_24h) {
            uint8_t pm = hr & 0x80;
            hr = bcd_to_bin((uint8_t)(hr & 0x7F));
            if (pm && hr < 12) hr = (uint8_t)(hr + 12);
            if (!pm && hr == 12) hr = 0;
        } else {
            hr = bcd_to_bin(hr);
        }
    } else {
        if (!is_24h) {
            uint8_t pm = hr & 0x80;
            hr = (uint8_t)(hr & 0x7F);
            if (pm && hr < 12) hr = (uint8_t)(hr + 12);
            if (!pm && hr == 12) hr = 0;
        }
    }

    *hours = hr;
    *minutes = min;
    *seconds = sec;
}

static void print_two_digits(uint8_t v) {
    char buf[3];
    buf[0] = (char)('0' + ((v / 10) % 10));
    buf[1] = (char)('0' + (v % 10));
    buf[2] = '\0';
    vga_print(buf, VGA_COLOR_LIGHT_CYAN);
}

/* ASCII Art for "Rayen OS" */
static const char* logo[] = {
"██████╗  █████╗ ██╗   ██╗███████╗███╗   ██╗     ██████╗ ███████╗",
"██╔══██╗██╔══██╗╚██╗ ██╔╝██╔════╝████╗  ██║    ██╔═══██╗██╔════╝",
"██████╔╝███████║ ╚████╔╝ █████╗  ██╔██╗ ██║    ██║   ██║███████╗",
"██╔══██╗██╔══██║  ╚██╔╝  ██╔══╝  ██║╚██╗██║    ██║   ██║╚════██║",
"██║  ██║██║  ██║   ██║   ███████╗██║ ╚████║    ╚██████╔╝███████║",
"╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═══╝     ╚═════╝ ╚══════╝"
};

void ui_init(void) {
    /* Initialize buttons */
    buttons[0].x = 28;
    buttons[0].y = 8;
    buttons[0].width = 24;
    buttons[0].height = 3;
    buttons[0].label = "   [1] Show Time     ";
    buttons[0].selected = 1;
    
    buttons[1].x = 28;
    buttons[1].y = 12;
    buttons[1].width = 24;
    buttons[1].height = 3;
    buttons[1].label = "   [2] Snake Game    ";
    buttons[1].selected = 0;
    
    buttons[2].x = 28;
    buttons[2].y = 16;
    buttons[2].width = 24;
    buttons[2].height = 3;
    buttons[2].label = "   [3] System Info   ";
    buttons[2].selected = 0;
    
    current_selection = 0;
    current_screen = UI_SCREEN_MENU;
    screen_dirty = 1;
}

static void draw_logo(void) {
    uint8_t start_y = 3;
    uint8_t start_x = 20;
    
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
    vga_print(btn->label, color);
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
    vga_set_cursor(18, 10);
    vga_print("Welcome to Mohamed Rayen Ouerghui OS", VGA_COLOR_LIGHT_GRAY);
    
    /* Draw buttons */
    for (int i = 0; i < NUM_BUTTONS; i++) {
        draw_button(&buttons[i]);
    }
    
    /* Draw instructions */
    vga_set_cursor(10, 27);
    vga_print("Keys: [1][2][3] Select   Snake: WASD or QZDS   Back: ESC or X", VGA_COLOR_DARK_GRAY);
}

void ui_handle_input(uint8_t scancode) {
    ui_last_scancode = scancode;

    /* ESC (0x01) or X (0x2D) always requests exit */
    if (scancode == 0x01 || scancode == 0x2D) {
        ui_exit_requested = 1;
    }

    if (current_screen != UI_SCREEN_MENU) {
        return;
    }
    
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
        
        /* Switch screen (actual rendering happens in ui_update in main loop) */
        ui_exit_requested = 0;
        ui_last_scancode = 0;
        if (choice == 0) current_screen = UI_SCREEN_TIME;
        else if (choice == 1) current_screen = UI_SCREEN_SNAKE;
        else current_screen = UI_SCREEN_SYSINFO;
        screen_dirty = 1;
    }
}

void ui_show_time(void) {
    vga_clear();
    
    /* Title */
    vga_set_cursor(30, 5);
    vga_print("=== SYSTEM TIME ===", VGA_COLOR_LIGHT_CYAN);
    
    vga_set_cursor(24, 9);
    vga_print("Current Time (RTC): ", VGA_COLOR_WHITE);

    vga_set_cursor(25, 11);
    vga_print("System Uptime:", VGA_COLOR_WHITE);

    /* Instructions */
    vga_set_cursor(25, 20);
    vga_print("Press ESC or X to return to menu...", VGA_COLOR_YELLOW);

    /* Rendering + refresh handled in ui_update() */
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
    
    snake_x = 40;
    snake_y = 12;
    snake_dir = 1;
    snake_last_tick = timer_get_ticks();
    
    vga_set_cursor(snake_x, snake_y);
    vga_print("@", VGA_COLOR_LIGHT_GREEN);
    
    /* Food */
    vga_set_cursor(50, 15);
    vga_print("*", VGA_COLOR_LIGHT_RED);
    
    /* Instructions */
    vga_set_cursor(12, 22);
    vga_print("Move: WASD / QZDS   |   Back: ESC or X", VGA_COLOR_YELLOW);
    
    vga_set_cursor(25, 23);
    vga_print("", VGA_COLOR_DARK_GRAY);
    
    /* Updates handled in ui_update() */
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
    vga_print("Press ESC or X to return to menu...", VGA_COLOR_YELLOW);
    
    /* Rendering handled in ui_update() */
}

void ui_update(void) {
    /* Exit handling */
    if (ui_exit_requested && current_screen != UI_SCREEN_MENU) {
        ui_exit_requested = 0;
        current_screen = UI_SCREEN_MENU;
        screen_dirty = 1;
    }

    if (screen_dirty) {
        screen_dirty = 0;
        if (current_screen == UI_SCREEN_MENU) {
            ui_draw_menu();
        } else if (current_screen == UI_SCREEN_TIME) {
            ui_show_time();
        } else if (current_screen == UI_SCREEN_SNAKE) {
            ui_show_snake();
        } else if (current_screen == UI_SCREEN_SYSINFO) {
            ui_show_sysinfo();
        }
    }

    if (current_screen == UI_SCREEN_TIME) {
        uint64_t ticks = timer_get_ticks();
        static uint64_t last_refresh = 0;
        if ((ticks - last_refresh) >= 10) {
            last_refresh = ticks;

            uint8_t h, m, s;
            rtc_read_time(&h, &m, &s);
            vga_set_cursor(44, 9);
            print_two_digits(h);
            vga_print(":", VGA_COLOR_LIGHT_CYAN);
            print_two_digits(m);

            uint64_t up_seconds = ticks / 100;
            uint64_t up_minutes = up_seconds / 60;
            uint64_t up_hours = up_minutes / 60;
            up_seconds %= 60;
            up_minutes %= 60;
            up_hours %= 24;

            vga_set_cursor(30, 13);
            vga_print("Hours:   ", VGA_COLOR_LIGHT_GRAY);
            print_number(up_hours);
            vga_print("   ", VGA_COLOR_LIGHT_GRAY);

            vga_set_cursor(30, 14);
            vga_print("Minutes: ", VGA_COLOR_LIGHT_GRAY);
            print_number(up_minutes);
            vga_print("   ", VGA_COLOR_LIGHT_GRAY);

            vga_set_cursor(30, 15);
            vga_print("Seconds: ", VGA_COLOR_LIGHT_GRAY);
            print_number(up_seconds);
            vga_print("   ", VGA_COLOR_LIGHT_GRAY);

            vga_set_cursor(30, 17);
            vga_print("Total Ticks: ", VGA_COLOR_LIGHT_GRAY);
            print_number(ticks);
            vga_print("   ", VGA_COLOR_LIGHT_GRAY);
        }
    } else if (current_screen == UI_SCREEN_SNAKE) {
        /* Handle WASD */
        uint8_t sc = ui_last_scancode;
        ui_last_scancode = 0;
        if (sc == 0x11 || sc == 0x2C) snake_dir = 0;      /* W or Z */
        else if (sc == 0x1F) snake_dir = 2;               /* S */
        else if (sc == 0x1E || sc == 0x10) snake_dir = 3; /* A or Q */
        else if (sc == 0x20) snake_dir = 1;               /* D */

        uint64_t now = timer_get_ticks();
        if ((now - snake_last_tick) >= snake_tick_interval) {
            snake_last_tick = now;
            vga_set_cursor(snake_x, snake_y);
            vga_print(" ", VGA_COLOR_BLACK);

            if (snake_dir == 0 && snake_y > 6) snake_y--;
            else if (snake_dir == 2 && snake_y < 19) snake_y++;
            else if (snake_dir == 3 && snake_x > 11) snake_x--;
            else if (snake_dir == 1 && snake_x < 68) snake_x++;

            vga_set_cursor(snake_x, snake_y);
            vga_print("@", VGA_COLOR_LIGHT_GREEN);
        }
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
