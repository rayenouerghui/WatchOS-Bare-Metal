#include <stdint.h>
#include <stddef.h>

#include "allocator.h"
#include "kprint.h"
#include "panic.h"
#include "idt.h"
#include "pic.h"

/* .data section */
uint32_t kernel_version = 1;

/* .bss section */
uint32_t boot_count;

void kernel_main(void) {
    /* Initialize VGA + logging */
    kprint_init();

    /* Phase 3.1: Initialize IDT EARLY */
    kprint_info("Initializing IDT...");
    idt_init();
    kprint_ok("IDT loaded successfully");

    /* Phase 3.4: Remap PIC */
    kprint_info("Remapping PIC...");
    pic_remap();
    kprint_ok("PIC remapped");

    /* Boot messages */
    kprint_ok("WatchOS Kernel v1.0 booted successfully");
    kprint_info("Initializing kernel subsystems...");

    /* Memory sections */
    kprint_info(".data section initialized");
    kprint_info(".bss section initialized");

    /* Phase 2: dynamic memory test */
    kprint_info("Testing dynamic memory allocation...");

    uint8_t* ptr1 = (uint8_t*)kmalloc(16);
    uint8_t* ptr2 = (uint8_t*)kmalloc(32);

    if (!ptr1 || !ptr2) {
        panic("Memory allocation failed!");
    }

    for (int i = 0; i < 16; i++) ptr1[i] = 'A' + i;
    for (int i = 0; i < 32; i++) ptr2[i] = 'a' + i;

    kprint_ok("Memory allocation test passed");
    kprint_info("Allocated 48 bytes total");

    kprint_ok("All systems operational");

    /* Phase 3.5: Enable keyboard */
    kprint_info("Enabling keyboard interrupt...");
    pic_unmask_irq1();
    __asm__ volatile ("sti");
    kprint_ok("Interrupts enabled - press keys!");

    kprint_info("Kernel idle - halting CPU");

    while (1) {
        __asm__ volatile ("hlt");
    }
}

