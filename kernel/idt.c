#include "idt.h"

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idt_descriptor;

extern void idt_load(struct idt_ptr*);

static void idt_set_entry(
    int vector,
    uint64_t handler,
    uint8_t type_attr
) {
    idt[vector].offset_low  = handler & 0xFFFF;
    idt[vector].selector    = 0x08;   // kernel code segment
    idt[vector].ist         = 0;
    idt[vector].type_attr  = type_attr;
    idt[vector].offset_mid = (handler >> 16) & 0xFFFF;
    idt[vector].offset_high= (handler >> 32) & 0xFFFFFFFF;
    idt[vector].zero        = 0;
}

void idt_init(void) {
    // Zero the IDT for now
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i] = (struct idt_entry){0};
    }

    extern void isr_default(void);
    extern void irq1_handler(void);

    for (int i = 0; i < 32; i++) {
        idt_set_entry(i, (uint64_t)isr_default, 0x8E);
    }

    idt_set_entry(33, (uint64_t)irq1_handler, 0x8E);

    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base  = (uint64_t)&idt;

    idt_load(&idt_descriptor);
}
