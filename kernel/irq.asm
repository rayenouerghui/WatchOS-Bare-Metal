global irq1_handler
extern keyboard_handler

irq1_handler:
    pushaq
    call keyboard_handler
    popaq
    mov al, 0x20
    out 0x20, al
    iretq
