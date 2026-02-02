ENTRY(_start)

SECTIONS
{
    . = 0x100000; /* load address 1MB */

    /* Multiboot header MUST be first */
    .multiboot : ALIGN(8) {
        *(.multiboot)
    }

    .text : ALIGN(4096) {
        *(.text)
    }
    
    .rodata : ALIGN(4096) {
        *(.rodata)
    }
    
    .data : ALIGN(4096) {
        *(.data)
    }
    
    .bss : ALIGN(4096) {
        *(.bss)
        *(COMMON)
    }

    /* Heap after BSS */
    . = ALIGN(4096);
    heap_start = .;
    . += 0x10000; /* 64KB heap */
    heap_end = .;
}


