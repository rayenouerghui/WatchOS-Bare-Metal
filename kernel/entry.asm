section .bss
align 16
stack: resb 65536        ; 64KB stack

section .text
global _start
extern kernel_main

_start:
    lea rsp, [stack + 65536] ; top of stack
    call kernel_main

hang:
    jmp hang

