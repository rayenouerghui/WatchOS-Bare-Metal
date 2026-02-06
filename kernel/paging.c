#include "paging.h"
#include "pmm.h"
#include "panic.h"
#include "kprint.h"

#define ENTRIES_PER_TABLE 512
#define PAGE_TABLE_MASK 0x1FF
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL

/* Root page table (PML4) */
static pte_t* pml4_table = NULL;

/* Extract page table indices from virtual address */
static inline uint64_t pml4_index(uint64_t vaddr) {
    return (vaddr >> 39) & PAGE_TABLE_MASK;
}

static inline uint64_t pdpt_index(uint64_t vaddr) {
    return (vaddr >> 30) & PAGE_TABLE_MASK;
}

static inline uint64_t pd_index(uint64_t vaddr) {
    return (vaddr >> 21) & PAGE_TABLE_MASK;
}

static inline uint64_t pt_index(uint64_t vaddr) {
    return (vaddr >> 12) & PAGE_TABLE_MASK;
}

/* Get or create a page table */
static pte_t* get_or_create_table(pte_t* parent_table, uint64_t index) {
    if (parent_table[index] & PAGE_PRESENT) {
        /* Table exists, return it */
        return (pte_t*)(parent_table[index] & PAGE_ADDR_MASK);
    }
    
    /* Allocate new table */
    uint64_t new_table_phys = pmm_alloc_page();
    pte_t* new_table = (pte_t*)new_table_phys;
    
    /* Zero out the new table */
    for (int i = 0; i < ENTRIES_PER_TABLE; i++) {
        new_table[i] = 0;
    }
    
    /* Install in parent */
    parent_table[index] = new_table_phys | PAGE_PRESENT | PAGE_WRITE;
    
    return new_table;
}

void paging_init(void) {
    /* Allocate PML4 table */
    pml4_table = (pte_t*)pmm_alloc_page();
    
    /* Zero out PML4 */
    for (int i = 0; i < ENTRIES_PER_TABLE; i++) {
        pml4_table[i] = 0;
    }
    
    /* Identity map first 4MB (kernel space) */
    for (uint64_t addr = 0; addr < 0x400000; addr += PAGE_SIZE) {
        paging_map_page(addr, addr, PAGE_PRESENT | PAGE_WRITE);
    }
    
    kprint_info("Paging initialized (identity mapped 4MB)");
}

void paging_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
    /* Get indices */
    uint64_t pml4_i = pml4_index(virt_addr);
    uint64_t pdpt_i = pdpt_index(virt_addr);
    uint64_t pd_i = pd_index(virt_addr);
    uint64_t pt_i = pt_index(virt_addr);
    
    /* Walk page tables, creating as needed */
    pte_t* pdpt = get_or_create_table(pml4_table, pml4_i);
    pte_t* pd = get_or_create_table(pdpt, pdpt_i);
    pte_t* pt = get_or_create_table(pd, pd_i);
    
    /* Map the page */
    pt[pt_i] = (phys_addr & PAGE_ADDR_MASK) | flags;
}

void paging_unmap_page(uint64_t virt_addr) {
    uint64_t pml4_i = pml4_index(virt_addr);
    uint64_t pdpt_i = pdpt_index(virt_addr);
    uint64_t pd_i = pd_index(virt_addr);
    uint64_t pt_i = pt_index(virt_addr);
    
    /* Walk to page table */
    if (!(pml4_table[pml4_i] & PAGE_PRESENT)) return;
    pte_t* pdpt = (pte_t*)(pml4_table[pml4_i] & PAGE_ADDR_MASK);
    
    if (!(pdpt[pdpt_i] & PAGE_PRESENT)) return;
    pte_t* pd = (pte_t*)(pdpt[pdpt_i] & PAGE_ADDR_MASK);
    
    if (!(pd[pd_i] & PAGE_PRESENT)) return;
    pte_t* pt = (pte_t*)(pd[pd_i] & PAGE_ADDR_MASK);
    
    /* Unmap */
    pt[pt_i] = 0;
    
    /* Invalidate TLB entry */
    __asm__ volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
}

uint64_t paging_get_physical(uint64_t virt_addr) {
    uint64_t pml4_i = pml4_index(virt_addr);
    uint64_t pdpt_i = pdpt_index(virt_addr);
    uint64_t pd_i = pd_index(virt_addr);
    uint64_t pt_i = pt_index(virt_addr);
    
    if (!(pml4_table[pml4_i] & PAGE_PRESENT)) return 0;
    pte_t* pdpt = (pte_t*)(pml4_table[pml4_i] & PAGE_ADDR_MASK);
    
    if (!(pdpt[pdpt_i] & PAGE_PRESENT)) return 0;
    pte_t* pd = (pte_t*)(pdpt[pdpt_i] & PAGE_ADDR_MASK);
    
    if (!(pd[pd_i] & PAGE_PRESENT)) return 0;
    pte_t* pt = (pte_t*)(pd[pd_i] & PAGE_ADDR_MASK);
    
    if (!(pt[pt_i] & PAGE_PRESENT)) return 0;
    
    return (pt[pt_i] & PAGE_ADDR_MASK) | (virt_addr & 0xFFF);
}

void paging_enable(void) {
    /* Load PML4 into CR3 */
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4_table));
    kprint_ok("Paging enabled (CR3 loaded)");
}
