SECTION .multiboot
align 8
multiboot_header:
    dd 0xe85250d6        ; multiboot2 magic
    dd 0                 ; architecture (i386)
    dd header_end - multiboot_header
    dd -(0xe85250d6 + 0 + (header_end - multiboot_header))

header_end:
BITS 64

global _start

extern stack_top
extern kernel_main

_start:
    mov rsp, stack_top
    call kernel_main

hang:
    jmp hang

