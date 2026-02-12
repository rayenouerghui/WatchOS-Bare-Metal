#include <stdint.h>
#include <stddef.h>

#include "allocator.h"
#include "kprint.h"
#include "panic.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "process.h"
#include "scheduler.h"
#include "vga.h"

/* Test process functions */
void process_a(void) {
    while (1) {
        vga_print("A", VGA_COLOR_LIGHT_GREEN);
        for (volatile int i = 0; i < 10000000; i++);  /* Busy wait */
    }
}

void process_b(void) {
    while (1) {
        vga_print("B", VGA_COLOR_LIGHT_CYAN);
        for (volatile int i = 0; i < 10000000; i++);  /* Busy wait */
    }
}

void process_c(void) {
    while (1) {
        vga_print("C", VGA_COLOR_YELLOW);
        for (volatile int i = 0; i < 10000000; i++);  /* Busy wait */
    }
}

void kernel_main(void) {
    /* Step 1: Initialize VGA first for debugging output */
    kprint_init();
    kprint_info("WatchOS Kernel starting...");
    kprint_info("");
    
    /* Step 2: Initialize Interrupt Descriptor Table (but don't enable yet) */
    kprint_info("Setting up IDT...");
    idt_init();
    kprint_ok("IDT initialized (256 entries)");
    
    /* Step 3: Remap PIC to avoid conflicts with CPU exceptions */
    kprint_info("Remapping PIC...");
    pic_remap();
    kprint_ok("PIC remapped (IRQ0-15 -> INT 32-47)");
    
    /* Step 4: Initialize Physical Memory Manager BEFORE paging */
    kprint_info("Initializing Physical Memory Manager...");
    pmm_init(32 * 1024 * 1024);  // Assume 32MB RAM
    kprint_ok("PMM initialized");
    
    /* Step 5: Initialize Paging */
    kprint_info("Setting up paging...");
    paging_init();
    paging_enable();
    
    /* Step 6: Initialize Heap Allocator */
    kprint_info("Initializing heap allocator...");
    heap_init();
    
    kprint_info("");
    kprint_info("=== Phase 5: Multitasking ===");
    kprint_info("");
    
    /* Step 7: Initialize Process Management */
    kprint_info("Initializing process management...");
    process_init();
    
    /* Step 8: Initialize Scheduler */
    kprint_info("Initializing scheduler...");
    scheduler_init();
    
    /* Step 9: Create test processes */
    kprint_info("Creating test processes...");
    process_t* proc_a = process_create(process_a, 8192);
    process_t* proc_b = process_create(process_b, 8192);
    process_t* proc_c = process_create(process_c, 8192);
    
    scheduler_add(proc_a);
    scheduler_add(proc_b);
    scheduler_add(proc_c);
    kprint_ok("Created 3 test processes (A, B, C)");
    
    /* Step 10: NOW enable interrupts (after everything is set up) */
    kprint_info("Initializing timer (100 Hz)...");
    timer_init(100);
    pic_unmask_irq0();
    kprint_ok("Timer initialized and enabled");
    
    kprint_info("Enabling keyboard interrupt...");
    pic_unmask_irq1();
    kprint_ok("Keyboard IRQ enabled");
    
    kprint_info("Enabling interrupts...");
    __asm__ volatile("sti");
    kprint_ok("Interrupts enabled");
    
    /* Boot complete */
    kprint_info("");
    kprint_ok("WatchOS Kernel booted successfully!");
    kprint_info("");
    kprint_ok("Phase 5 Complete - Multitasking Operational");
    kprint_info("Watch the A, B, C characters appear (round-robin scheduling)");
    kprint_info("");
    
    /* Idle loop - wait for interrupts */
    while (1) {
        __asm__ volatile("hlt");  // Halt until next interrupt
    }
}
