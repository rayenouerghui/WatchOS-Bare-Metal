#include <stdint.h>
#include <stddef.h>

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
#include "ui.h"

void kernel_main(void) {
    /* Initialize VGA */
    kprint_init();
    
    /* Initialize IDT */
    idt_init();
    
    /* Remap PIC */
    pic_remap();
    
    /* Initialize Memory Management */
    pmm_init(32 * 1024 * 1024);
    paging_init();
    paging_enable();
    heap_init();
    
    /* Initialize Process Management */
    process_init();
    scheduler_init();
    
    /* Initialize UI */
    ui_init();
    
    /* Enable interrupts */
    timer_init(100);
    pic_unmask_irq0();
    pic_unmask_irq1();
    __asm__ volatile("sti");
    
    /* Show the main menu */
    ui_draw_menu();
    
    /* Idle loop */
    while (1) {
        __asm__ volatile("hlt");
        ui_update();
    }
}
