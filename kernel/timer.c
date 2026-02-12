#include "timer.h"
#include "scheduler.h"
#include <stdint.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_BASE_FREQ 1193182

static volatile uint64_t timer_ticks = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void timer_init(uint32_t frequency) {
    /* Calculate divisor */
    uint32_t divisor = PIT_BASE_FREQ / frequency;
    
    /* Send command byte: channel 0, lo/hi byte, rate generator */
    outb(PIT_COMMAND, 0x36);
    
    /* Send frequency divisor */
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
}

void timer_handler(void) {
    timer_ticks++;
    
    /* Call scheduler every tick for multitasking */
    scheduler_switch();
}

uint64_t timer_get_ticks(void) {
    return timer_ticks;
}
