#include "allocator.h"
#include "panic.h"
#include <stdint.h>
#include <stddef.h>

extern uint8_t heap_start;
extern uint8_t heap_end;

static uint8_t* heap_ptr = &heap_start;

void* kmalloc(size_t size) {
    if (heap_ptr + size > &heap_end) {
        // Out of memory - trigger kernel panic
        panic("Out of memory in kmalloc()");
    }
    void* ptr = heap_ptr;
    heap_ptr += size;
    return ptr;
}

