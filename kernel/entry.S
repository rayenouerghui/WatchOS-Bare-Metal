.section .multiboot,"a"
.balign 8
.global multiboot_header_start
multiboot_header_start:
    .long 0xe85250d6
    .long 0
    .long multiboot_header_end - multiboot_header_start
    .long -(0xe85250d6 + (multiboot_header_end - multiboot_header_start))
    .short 0
    .short 0
    .long 8
multiboot_header_end:
.section .bss,"aw",@nobits
.balign 16
stack_bottom: .skip 65536
stack_top:
.balign 4096
pml4_table: .skip 4096
pdp_table: .skip 4096
pd_table: .skip 4096
.section .rodata,"a"
.balign 8
gdt64:
    .quad 0
    .quad 0x00AF9A000000FFFF
    .quad 0x00AF92000000FFFF
gdt64_end:
gdt64_descriptor:
    .word gdt64_end - gdt64 - 1
    .quad gdt64
.section .text,"ax"
.global _start
.extern kernel_main
.code32
_start:
    cli
    cld
    xorl %eax, %eax
    movl $pml4_table, %edi
    movl $1024, %ecx
    rep stosl
    movl $pdp_table, %edi
    movl $1024, %ecx
    rep stosl
    movl $pd_table, %edi
    movl $1024, %ecx
    rep stosl
    movl $pdp_table, %eax
    orl $3, %eax
    movl %eax, pml4_table
    movl $0, pml4_table + 4
    movl $pd_table, %eax
    orl $3, %eax
    movl %eax, pdp_table
    movl $0, pdp_table + 4
    xorl %esi, %esi
    movl $pd_table, %edi
    movl $32, %ecx
.Lmap_pd_loop:
    movl %esi, %eax
    orl $0x83, %eax
    movl %eax, (%edi)
    movl $0, 4(%edi)
    addl $0x200000, %esi
    addl $8, %edi
    loop .Lmap_pd_loop
    lgdt gdt64_descriptor
    movl %cr4, %eax
    orl $(1 << 5), %eax
    movl %eax, %cr4
    movl $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr
    movl $pml4_table, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl $(1 << 31), %eax
    movl %eax, %cr0
    ljmp $0x08, $long_mode_start
.code64
long_mode_start:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs
    mov $stack_top, %rsp
    and $-16, %rsp
    xor %rax, %rax
    xor %rbx, %rbx
    xor %rcx, %rcx
    xor %rdx, %rdx
    call kernel_main
.Lhang:
    cli
    hlt
    jmp .Lhang
.section .note.GNU-stack,"",@progbits
