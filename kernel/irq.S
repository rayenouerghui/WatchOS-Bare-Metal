.intel_syntax noprefix
.section .text,"ax"
.extern timer_handler
.extern keyboard_handler
.extern pic_send_eoi
.macro IRQ n, handler
.global irq\n\()_handler
irq\n\()_handler:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    /* Keep the interrupted stack in callee-saved RBX across C calls. */
    mov rbx, rsp
    and rsp, -16
    call \handler
    mov rsp, rbx
    mov rdi, \n
    and rsp, -16
    call pic_send_eoi
    mov rsp, rbx
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq
.endm
IRQ 0, timer_handler
IRQ 1, keyboard_handler
IRQ 2, irq_default
IRQ 3, irq_default
IRQ 4, irq_default
IRQ 5, irq_default
IRQ 6, irq_default
IRQ 7, irq_default
IRQ 8, irq_default
IRQ 9, irq_default
IRQ 10, irq_default
IRQ 11, irq_default
IRQ 12, irq_default
IRQ 13, irq_default
IRQ 14, irq_default
IRQ 15, irq_default
irq_default:
    ret
.section .note.GNU-stack,"",@progbits
