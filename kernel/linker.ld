ENTRY(_start)

SECTIONS
{
    . = 0x100000; /* load address 1MB */

    .text : { *(.multiboot) *(.text) }   /* include multiboot header first */
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }

    heap_start = .;
    .heap : { *(.heap) }
    heap_end = heap_start + 0x10000; /* 64KB heap */

    stack_top = heap_end + 0x10000;  /* stack 64KB */
}

