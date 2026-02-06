#include "pmm.h"
#include "panic.h"
#include "kprint.h"

/* Bitmap to track page allocation status */
static uint8_t* page_bitmap = NULL;
static uint64_t total_pages = 0;
static uint64_t used_pages = 0;

/* Memory region after kernel and heap */
extern uint8_t heap_end;
#define BITMAP_START ((uint64_t)&heap_end)

/* Set a bit in the bitmap (mark page as used) */
static inline void bitmap_set(uint64_t page) {
    uint64_t byte = page / PAGES_PER_BYTE;
    uint64_t bit = page % PAGES_PER_BYTE;
    page_bitmap[byte] |= (1 << bit);
}

/* Clear a bit in the bitmap (mark page as free) */
static inline void bitmap_clear(uint64_t page) {
    uint64_t byte = page / PAGES_PER_BYTE;
    uint64_t bit = page % PAGES_PER_BYTE;
    page_bitmap[byte] &= ~(1 << bit);
}

/* Test if a bit is set in the bitmap */
static inline uint8_t bitmap_test(uint64_t page) {
    uint64_t byte = page / PAGES_PER_BYTE;
    uint64_t bit = page % PAGES_PER_BYTE;
    return (page_bitmap[byte] & (1 << bit)) != 0;
}

void pmm_init(uint64_t mem_size) {
    /* Calculate number of pages */
    total_pages = mem_size / PAGE_SIZE;
    
    /* Calculate bitmap size (1 bit per page) */
    uint64_t bitmap_size = (total_pages + PAGES_PER_BYTE - 1) / PAGES_PER_BYTE;
    
    /* Place bitmap right after heap */
    page_bitmap = (uint8_t*)BITMAP_START;
    
    /* Initialize bitmap - mark all pages as free */
    for (uint64_t i = 0; i < bitmap_size; i++) {
        page_bitmap[i] = 0;
    }
    
    /* Mark first 2MB as used (kernel, heap, bitmap) */
    uint64_t reserved_pages = (0x200000 + bitmap_size) / PAGE_SIZE + 1;
    for (uint64_t i = 0; i < reserved_pages; i++) {
        bitmap_set(i);
        used_pages++;
    }
    
    kprint_info("Physical Memory Manager initialized");
}

uint64_t pmm_alloc_page(void) {
    /* Find first free page */
    for (uint64_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            return i * PAGE_SIZE;
        }
    }
    
    /* Out of memory */
    panic("PMM: Out of physical memory");
    return 0;
}

void pmm_free_page(uint64_t page_addr) {
    uint64_t page = page_addr / PAGE_SIZE;
    
    if (page >= total_pages) {
        panic("PMM: Invalid page address");
    }
    
    if (!bitmap_test(page)) {
        panic("PMM: Double free detected");
    }
    
    bitmap_clear(page);
    used_pages--;
}

uint64_t pmm_get_free_memory(void) {
    return (total_pages - used_pages) * PAGE_SIZE;
}

uint64_t pmm_get_used_memory(void) {
    return used_pages * PAGE_SIZE;
}

uint64_t pmm_get_total_memory(void) {
    return total_pages * PAGE_SIZE;
}
