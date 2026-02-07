#include "heap.h"
#include "pmm.h"
#include "paging.h"
#include "panic.h"
#include "kprint.h"

#define HEAP_MAGIC 0xDEADBEEF
#define HEAP_START 0x10000000  /* 256MB virtual address */
#define HEAP_MAX_SIZE 0x1000000 /* 16MB max heap */
#define PAGE_SIZE 4096

/* Block header for allocated memory */
typedef struct block_header {
    uint32_t magic;
    uint32_t size;
    uint8_t is_free;
    struct block_header* next;
} block_header_t;

static block_header_t* heap_start = NULL;
static uint64_t heap_size = 0;

void heap_init(void) {
    /* Map initial heap pages (1MB) */
    for (uint64_t vaddr = HEAP_START; vaddr < HEAP_START + 0x100000; vaddr += PAGE_SIZE) {
        uint64_t paddr = pmm_alloc_page();
        paging_map_page(vaddr, paddr, PAGE_PRESENT | PAGE_WRITE);
    }
    
    /* Initialize first block */
    heap_start = (block_header_t*)HEAP_START;
    heap_start->magic = HEAP_MAGIC;
    heap_start->size = 0x100000 - sizeof(block_header_t);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    
    heap_size = 0x100000;
    
    kprint_ok("Heap allocator initialized (1MB at 0x10000000)");
}

void* heap_alloc(size_t size) {
    if (size == 0) return NULL;
    
    /* Align size to 16 bytes */
    size = (size + 15) & ~15;
    
    /* Find free block */
    block_header_t* current = heap_start;
    while (current) {
        if (current->magic != HEAP_MAGIC) {
            panic("Heap corruption detected");
        }
        
        if (current->is_free && current->size >= size) {
            /* Found suitable block */
            if (current->size > size + sizeof(block_header_t) + 16) {
                /* Split block */
                block_header_t* new_block = (block_header_t*)((uint8_t*)current + sizeof(block_header_t) + size);
                new_block->magic = HEAP_MAGIC;
                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = 0;
            return (void*)((uint8_t*)current + sizeof(block_header_t));
        }
        
        current = current->next;
    }
    
    /* No suitable block found */
    panic("Heap: Out of memory");
    return NULL;
}

void heap_free(void* ptr) {
    if (!ptr) return;
    
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    
    if (block->magic != HEAP_MAGIC) {
        panic("Heap: Invalid free (bad magic)");
    }
    
    if (block->is_free) {
        panic("Heap: Double free detected");
    }
    
    block->is_free = 1;
    
    /* Coalesce with next block if free */
    if (block->next && block->next->is_free) {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;
    }
    
    /* Coalesce with previous block if free */
    block_header_t* current = heap_start;
    while (current && current->next != block) {
        current = current->next;
    }
    
    if (current && current->is_free) {
        current->size += sizeof(block_header_t) + block->size;
        current->next = block->next;
    }
}

void heap_stats(uint64_t* total, uint64_t* used, uint64_t* free) {
    *total = heap_size;
    *used = 0;
    *free = 0;
    
    block_header_t* current = heap_start;
    while (current) {
        if (current->is_free) {
            *free += current->size;
        } else {
            *used += current->size;
        }
        current = current->next;
    }
}
