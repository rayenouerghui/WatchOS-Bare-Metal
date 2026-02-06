#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

/* Improved heap allocator with free() support
 * Uses a linked list of free blocks
 */

/* Initialize heap allocator */
void heap_init(void);

/* Allocate memory from heap */
void* heap_alloc(size_t size);

/* Free memory back to heap */
void heap_free(void* ptr);

/* Get heap statistics */
void heap_stats(uint64_t* total, uint64_t* used, uint64_t* free);

#endif
