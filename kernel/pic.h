#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* Remap PIC to avoid conflicts with CPU exceptions */
void pic_remap(void);

/* Disable all IRQs */
void pic_disable(void);

/* Enable specific IRQs */
void pic_unmask_irq0(void);  // Timer
void pic_unmask_irq1(void);  // Keyboard

/* Send End of Interrupt signal */
void pic_send_eoi(uint8_t irq);

#endif
