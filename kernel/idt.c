#include "idt.h"

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idt_descriptor;

extern void idt_load(struct idt_ptr*);

void idt_init(void) {
    // Zero the IDT for now
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i] = (struct idt_entry){0};
    }

    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base  = (uint64_t)&idt;

    idt_load(&idt_descriptor);
}
