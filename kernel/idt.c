#include "idt.h"
#include "kprint.h"

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idt_descriptor;

/* External assembly functions */
extern void idt_load(struct idt_ptr*);

/* Exception handlers (defined in exceptions.asm) */
extern void isr0(void);   // Divide by Zero
extern void isr1(void);   // Debug
extern void isr2(void);   // Non-Maskable Interrupt
extern void isr3(void);   // Breakpoint
extern void isr4(void);   // Overflow
extern void isr5(void);   // Bound Range Exceeded
extern void isr6(void);   // Invalid Opcode
extern void isr7(void);   // Device Not Available
extern void isr8(void);   // Double Fault
extern void isr9(void);   // Coprocessor Segment Overrun
extern void isr10(void);  // Invalid TSS
extern void isr11(void);  // Segment Not Present
extern void isr12(void);  // Stack-Segment Fault
extern void isr13(void);  // General Protection Fault
extern void isr14(void);  // Page Fault
extern void isr15(void);  // Reserved
extern void isr16(void);  // x87 Floating-Point Exception
extern void isr17(void);  // Alignment Check
extern void isr18(void);  // Machine Check
extern void isr19(void);  // SIMD Floating-Point Exception
extern void isr20(void);  // Virtualization Exception
extern void isr21(void);  // Control Protection Exception
extern void isr22(void);  // Reserved
extern void isr23(void);  // Reserved
extern void isr24(void);  // Reserved
extern void isr25(void);  // Reserved
extern void isr26(void);  // Reserved
extern void isr27(void);  // Reserved
extern void isr28(void);  // Reserved
extern void isr29(void);  // Reserved
extern void isr30(void);  // Reserved
extern void isr31(void);  // Reserved

/* IRQ handlers (defined in irq.asm) */
extern void irq0_handler(void);   // Timer
extern void irq1_handler(void);   // Keyboard
extern void irq2_handler(void);   // Cascade
extern void irq3_handler(void);   // COM2
extern void irq4_handler(void);   // COM1
extern void irq5_handler(void);   // LPT2
extern void irq6_handler(void);   // Floppy
extern void irq7_handler(void);   // LPT1
extern void irq8_handler(void);   // RTC
extern void irq9_handler(void);   // Peripherals
extern void irq10_handler(void);  // Peripherals
extern void irq11_handler(void);  // Peripherals
extern void irq12_handler(void);  // PS/2 Mouse
extern void irq13_handler(void);  // FPU
extern void irq14_handler(void);  // Primary ATA
extern void irq15_handler(void);  // Secondary ATA

static void idt_set_entry(int vector, uint64_t handler, uint8_t type_attr) {
    idt[vector].offset_low  = handler & 0xFFFF;
    idt[vector].selector    = 0x08;   // Kernel code segment
    idt[vector].ist         = 0;
    idt[vector].type_attr   = type_attr;
    idt[vector].offset_mid  = (handler >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[vector].zero        = 0;
}

void idt_init(void) {
    /* Zero out the entire IDT */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i] = (struct idt_entry){0};
    }

    /* Install CPU exception handlers (0-31) */
    idt_set_entry(0, (uint64_t)isr0, 0x8E);
    idt_set_entry(1, (uint64_t)isr1, 0x8E);
    idt_set_entry(2, (uint64_t)isr2, 0x8E);
    idt_set_entry(3, (uint64_t)isr3, 0x8E);
    idt_set_entry(4, (uint64_t)isr4, 0x8E);
    idt_set_entry(5, (uint64_t)isr5, 0x8E);
    idt_set_entry(6, (uint64_t)isr6, 0x8E);
    idt_set_entry(7, (uint64_t)isr7, 0x8E);
    idt_set_entry(8, (uint64_t)isr8, 0x8E);
    idt_set_entry(9, (uint64_t)isr9, 0x8E);
    idt_set_entry(10, (uint64_t)isr10, 0x8E);
    idt_set_entry(11, (uint64_t)isr11, 0x8E);
    idt_set_entry(12, (uint64_t)isr12, 0x8E);
    idt_set_entry(13, (uint64_t)isr13, 0x8E);
    idt_set_entry(14, (uint64_t)isr14, 0x8E);
    idt_set_entry(15, (uint64_t)isr15, 0x8E);
    idt_set_entry(16, (uint64_t)isr16, 0x8E);
    idt_set_entry(17, (uint64_t)isr17, 0x8E);
    idt_set_entry(18, (uint64_t)isr18, 0x8E);
    idt_set_entry(19, (uint64_t)isr19, 0x8E);
    idt_set_entry(20, (uint64_t)isr20, 0x8E);
    idt_set_entry(21, (uint64_t)isr21, 0x8E);
    idt_set_entry(22, (uint64_t)isr22, 0x8E);
    idt_set_entry(23, (uint64_t)isr23, 0x8E);
    idt_set_entry(24, (uint64_t)isr24, 0x8E);
    idt_set_entry(25, (uint64_t)isr25, 0x8E);
    idt_set_entry(26, (uint64_t)isr26, 0x8E);
    idt_set_entry(27, (uint64_t)isr27, 0x8E);
    idt_set_entry(28, (uint64_t)isr28, 0x8E);
    idt_set_entry(29, (uint64_t)isr29, 0x8E);
    idt_set_entry(30, (uint64_t)isr30, 0x8E);
    idt_set_entry(31, (uint64_t)isr31, 0x8E);

    /* Install IRQ handlers (32-47) */
    idt_set_entry(32, (uint64_t)irq0_handler, 0x8E);   // Timer
    idt_set_entry(33, (uint64_t)irq1_handler, 0x8E);   // Keyboard
    idt_set_entry(34, (uint64_t)irq2_handler, 0x8E);
    idt_set_entry(35, (uint64_t)irq3_handler, 0x8E);
    idt_set_entry(36, (uint64_t)irq4_handler, 0x8E);
    idt_set_entry(37, (uint64_t)irq5_handler, 0x8E);
    idt_set_entry(38, (uint64_t)irq6_handler, 0x8E);
    idt_set_entry(39, (uint64_t)irq7_handler, 0x8E);
    idt_set_entry(40, (uint64_t)irq8_handler, 0x8E);
    idt_set_entry(41, (uint64_t)irq9_handler, 0x8E);
    idt_set_entry(42, (uint64_t)irq10_handler, 0x8E);
    idt_set_entry(43, (uint64_t)irq11_handler, 0x8E);
    idt_set_entry(44, (uint64_t)irq12_handler, 0x8E);
    idt_set_entry(45, (uint64_t)irq13_handler, 0x8E);
    idt_set_entry(46, (uint64_t)irq14_handler, 0x8E);
    idt_set_entry(47, (uint64_t)irq15_handler, 0x8E);

    /* Load the IDT */
    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base  = (uint64_t)&idt;
    idt_load(&idt_descriptor);
}
