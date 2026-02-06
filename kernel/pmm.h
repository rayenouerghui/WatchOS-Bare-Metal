#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

/* Physical Memory Manager
 * Manages physical memory pages (4KB each)
 * Uses a bitmap to track free/used pages
 */

#define PAGE_SIZE 4096
#define PAGES_PER_BYTE 8

/* Initialize physical memory manager */
void pmm_init(uint64_t mem_size);

/* Allocate a physical page (returns physical address) */
uint64_t pmm_alloc_page(void);

/* Free a physical page */
void pmm_free_page(uint64_t page_addr);

/* Get memory statistics */
uint64_t pmm_get_free_memory(void);
uint64_t pmm_get_used_memory(void);
uint64_t pmm_get_total_memory(void);

#endif
