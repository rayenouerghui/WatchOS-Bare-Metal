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

align 4096
pml4_table:
    resq 512
pdp_table:
    resq 512
pd_table:
    resq 512

section .rodata
align 8
gdt64:
    dq 0
    dq 0x00AF9A000000FFFF
    dq 0x00AF92000000FFFF
gdt64_end:
gdt64_descriptor:
    dw gdt64_end - gdt64 - 1
    dq gdt64

section .text
global _start
extern kernel_main

_start:
    cli
    cld

    bits 32
    ; Zero page tables (BSS is not guaranteed to be zeroed by the bootloader)
    xor eax, eax
    mov edi, pml4_table
    mov ecx, 1024
    rep stosd
    mov edi, pdp_table
    mov ecx, 1024
    rep stosd
    mov edi, pd_table
    mov ecx, 1024
    rep stosd

    ; Build minimal identity-mapped paging structures (first 64 MiB)
    mov eax, pdp_table
    or eax, 0x03
    mov dword [pml4_table], eax
    mov dword [pml4_table + 4], 0
    mov eax, pd_table
    or eax, 0x03
    mov dword [pdp_table], eax
    mov dword [pdp_table + 4], 0

    xor esi, esi
    mov edi, pd_table
    mov ecx, 32
.map_pd_loop:
    mov eax, esi
    or eax, 0x83
    mov dword [edi], eax
    mov dword [edi + 4], 0
    add esi, 0x200000
    add edi, 8
    loop .map_pd_loop

    ; Load our GDT
    lgdt [gdt64_descriptor]

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Load PML4
    mov eax, pml4_table
    mov cr3, eax

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Far jump to 64-bit code segment
    jmp 0x08:long_mode_start

long_mode_start:
    bits 64
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov rsp, stack_top
    and rsp, -16

    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx

    call kernel_main

hang:
    cli
    hlt
    jmp hang


