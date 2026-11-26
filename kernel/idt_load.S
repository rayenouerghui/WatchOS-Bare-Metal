.intel_syntax noprefix
.section .text,"ax"
.global idt_load
idt_load:
    lidt [rdi]
    ret
.section .note.GNU-stack,"",@progbits
