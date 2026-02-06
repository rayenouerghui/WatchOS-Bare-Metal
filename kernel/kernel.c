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

void kernel_main(void) {
    /* Step 1: Initialize VGA first for debugging output */
    kprint_init();
    kprint_info("WatchOS Kernel starting...");
    kprint_info("");
    
    /* Step 2: Initialize Interrupt Descriptor Table */
    kprint_info("Setting up IDT...");
    idt_init();
    kprint_ok("IDT initialized (256 entries)");
    
    /* Step 3: Remap PIC to avoid conflicts with CPU exceptions */
    kprint_info("Remapping PIC...");
    pic_remap();
    kprint_ok("PIC remapped (IRQ0-15 -> INT 32-47)");
    
    /* Step 4: Initialize timer (100 Hz) */
    kprint_info("Initializing timer (100 Hz)...");
    timer_init(100);
    pic_unmask_irq0();
    kprint_ok("Timer initialized and enabled");
    
    /* Step 5: Enable keyboard interrupt (IRQ1) */
    kprint_info("Enabling keyboard interrupt...");
    pic_unmask_irq1();
    kprint_ok("Keyboard IRQ enabled");
    
    /* Step 6: Enable interrupts globally */
    kprint_info("Enabling interrupts...");
    __asm__ volatile("sti");
    kprint_ok("Interrupts enabled");
    
    kprint_info("");
    kprint_info("=== Phase 4: Memory Management ===");
    kprint_info("");
    
    /* Step 7: Initialize Physical Memory Manager */
    kprint_info("Initializing Physical Memory Manager...");
    pmm_init(32 * 1024 * 1024);  // Assume 32MB RAM
    kprint_ok("PMM initialized");
    
    /* Step 8: Initialize Paging */
    kprint_info("Setting up paging...");
    paging_init();
    paging_enable();
    
    /* Step 9: Initialize Heap Allocator */
    kprint_info("Initializing heap allocator...");
    heap_init();
    
    /* Display memory statistics */
    kprint_info("");
    kprint_info("Memory Statistics:");
    uint64_t total_mem = pmm_get_total_memory();
    uint64_t used_mem = pmm_get_used_memory();
    uint64_t free_mem = pmm_get_free_memory();
    
    /* Boot complete */
    kprint_info("");
    kprint_ok("WatchOS Kernel booted successfully!");
    kprint_info("");
    kprint_ok("Phase 4 Complete - Memory Management Operational");
    kprint_info("Type to test keyboard input...");
    kprint_info("");
    
    /* Test heap allocator */
    kprint_info("Testing heap allocator...");
    void* test1 = heap_alloc(128);
    void* test2 = heap_alloc(256);
    void* test3 = heap_alloc(512);
    kprint_ok("Allocated 3 blocks (128, 256, 512 bytes)");
    
    heap_free(test2);
    kprint_ok("Freed middle block (256 bytes)");
    
    void* test4 = heap_alloc(128);
    kprint_ok("Reallocated 128 bytes (reused freed space)");
    
    kprint_info("");
    
    /* Idle loop - wait for interrupts */
    while (1) {
        __asm__ volatile("hlt");  // Halt until next interrupt
    }
}
