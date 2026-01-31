ENTRY(_start)

SECTIONS
{
    . = 0x100000; /* load address 1MB */

    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }

    stack_top = . + 0x10000; /* stack 64KB */
}
