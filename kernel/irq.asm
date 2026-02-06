; IRQ Handlers (IRQ 0-15 mapped to interrupts 32-47)

global irq0_handler, irq1_handler, irq2_handler, irq3_handler
global irq4_handler, irq5_handler, irq6_handler, irq7_handler
global irq8_handler, irq9_handler, irq10_handler, irq11_handler
global irq12_handler, irq13_handler, irq14_handler, irq15_handler

extern timer_handler
extern keyboard_handler
extern pic_send_eoi

; Macro for IRQ handlers
%macro IRQ_HANDLER 2
irq%1_handler:
    ; Save all registers
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

    ; Call the C handler
    call %2

    ; Send End of Interrupt to PIC
    mov rdi, %1
    call pic_send_eoi

    ; Restore all registers
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

    ; Return from interrupt
    iretq
%endmacro

; IRQ Handlers
IRQ_HANDLER 0, timer_handler        ; PIT Timer
IRQ_HANDLER 1, keyboard_handler     ; Keyboard
IRQ_HANDLER 2, irq_default          ; Cascade (never raised)
IRQ_HANDLER 3, irq_default          ; COM2
IRQ_HANDLER 4, irq_default          ; COM1
IRQ_HANDLER 5, irq_default          ; LPT2
IRQ_HANDLER 6, irq_default          ; Floppy
IRQ_HANDLER 7, irq_default          ; LPT1
IRQ_HANDLER 8, irq_default          ; RTC
IRQ_HANDLER 9, irq_default          ; Peripherals
IRQ_HANDLER 10, irq_default         ; Peripherals
IRQ_HANDLER 11, irq_default         ; Peripherals
IRQ_HANDLER 12, irq_default         ; PS/2 Mouse
IRQ_HANDLER 13, irq_default         ; FPU
IRQ_HANDLER 14, irq_default         ; Primary ATA
IRQ_HANDLER 15, irq_default         ; Secondary ATA

; Default IRQ handler (does nothing)
irq_default:
    ret
