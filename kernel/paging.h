#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

/* x86_64 Paging Structures
 * 4-level paging: PML4 -> PDPT -> PD -> PT
 * Each page table has 512 entries (64-bit each)
 */

#define PAGE_PRESENT    (1ULL << 0)
#define PAGE_WRITE      (1ULL << 1)
#define PAGE_USER       (1ULL << 2)
#define PAGE_SIZE_FLAG  (1ULL << 7)

/* Page table entry */
typedef uint64_t pte_t;

/* Initialize paging system */
void paging_init(void);

/* Map a virtual address to a physical address */
void paging_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);

/* Unmap a virtual address */
void paging_unmap_page(uint64_t virt_addr);

/* Get physical address from virtual address */
uint64_t paging_get_physical(uint64_t virt_addr);

/* Enable paging (load CR3) */
void paging_enable(void);

#endif
