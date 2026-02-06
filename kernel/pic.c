#include "pic.h"
#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

#define PIC_EOI 0x20

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pic_remap(void) {
    uint8_t mask1, mask2;
    
    /* Save current masks */
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);
    
    /* Start initialization sequence (ICW1) */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    /* Set new interrupt offsets (ICW2) */
    outb(PIC1_DATA, 0x20);  // IRQ0-7 → Interrupts 32-39
    outb(PIC2_DATA, 0x28);  // IRQ8-15 → Interrupts 40-47

    /* Tell PICs how they are wired (ICW3) */
    outb(PIC1_DATA, 0x04);  // Slave PIC at IRQ2
    outb(PIC2_DATA, 0x02);  // Slave identity

    /* Set 8086 mode (ICW4) */
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    /* Restore saved masks */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_disable(void) {
    /* Mask all IRQs */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_unmask_irq1(void) {
    /* Enable IRQ1 (keyboard) only */
    uint8_t mask = inb(PIC1_DATA);
    mask &= ~(1 << 1);  // Clear bit 1 (IRQ1)
    outb(PIC1_DATA, mask);
}

void pic_unmask_irq0(void) {
    /* Enable IRQ0 (timer) */
    uint8_t mask = inb(PIC1_DATA);
    mask &= ~(1 << 0);  // Clear bit 0 (IRQ0)
    outb(PIC1_DATA, mask);
}

void pic_send_eoi(uint8_t irq) {
    /* If IRQ came from slave PIC, send EOI to both */
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    /* Always send EOI to master PIC */
    outb(PIC1_COMMAND, PIC_EOI);
}
