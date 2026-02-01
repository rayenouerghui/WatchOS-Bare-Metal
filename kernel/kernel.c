#include <stdint.h>
#include <stddef.h>
#include "allocator.h"

/* .data section (initialized global) */
uint32_t kernel_version = 1;

/* .bss section (uninitialized global) */
uint32_t boot_count;

/* VGA text buffer */
static volatile char* const VGA = (char*)0xB8000;

void kernel_main(void) {
    /* Show memory sections first */
    VGA[0] = 'D';  VGA[1] = 0x0F;  // .data exists
    VGA[2] = 'B';  VGA[3] = 0x0F;  // .bss exists
    VGA[4] = 'K';  VGA[5] = 0x0F;  // kernel running

    /* --- Phase 2.2: dynamic memory test --- */
    uint8_t* ptr1 = (uint8_t*)kmalloc(16);   // allocate 16 bytes
    uint8_t* ptr2 = (uint8_t*)kmalloc(32);   // allocate 32 bytes

    /* fill memory */
    for (int i = 0; i < 16; i++) ptr1[i] = 'A' + i;
    for (int i = 0; i < 32; i++) ptr2[i] = 'a' + i;

    /* copy first few bytes of ptr1 and ptr2 to VGA */
    for (int i = 0; i < 8; i++) {
        VGA[6 + i*2] = ptr1[i];    // char
        VGA[6 + i*2 + 1] = 0x0F;   // color
    }
    for (int i = 0; i < 8; i++) {
        VGA[22 + i*2] = ptr2[i];   // char
        VGA[22 + i*2 + 1] = 0x0F;  // color
    }

    /* halt */
    while (1) { __asm__("hlt"); }
}

