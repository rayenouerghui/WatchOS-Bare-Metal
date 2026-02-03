#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void pic_remap(void) {
    /* Start initialization */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    /* Set new offsets */
    outb(PIC1_DATA, 0x20); // IRQ0 → 32
    outb(PIC2_DATA, 0x28); // IRQ8 → 40

    /* Tell PICs how they are wired */
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    /* Set 8086 mode */
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    /* Mask all IRQs for now */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_unmask_irq1(void) {
    outb(PIC1_DATA, 0xFD); // enable IRQ1 only
}
