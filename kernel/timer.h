#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* Initialize PIT timer */
void timer_init(uint32_t frequency);

/* Timer interrupt handler (called from IRQ0) */
void timer_handler(void);

/* Get current tick count */
uint64_t timer_get_ticks(void);

#endif
