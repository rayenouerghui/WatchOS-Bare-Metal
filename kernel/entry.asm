; Multiboot2 header - MUST be at the very start
section .multiboot
align 8
multiboot_header_start:
    dd 0xe85250d6                ; magic number
    dd 0                         ; architecture (0 = i386)
    dd multiboot_header_end - multiboot_header_start  ; header length
    dd -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header_start))  ; checksum

    ; End tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_header_end:

section .bss
align 16
stack_bottom:
    resb 65536        ; 64KB stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Set up stack
    mov rsp, stack_top
    
    ; Call kernel main
    call kernel_main

hang:
    cli
    hlt
    jmp hang


