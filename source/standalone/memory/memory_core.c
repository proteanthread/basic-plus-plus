/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: memory_core.c
 * Subsystem: Interpreter Core Memory Allocator
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Bump allocator managing variables, arrays, and program storage.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed-memory allocations without using malloc/free.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Workspace memory limits, alignment scales.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Allocator bump strategy.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If allocation fails, check memory pool sizes in config.h.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE MEMORY POOL SUBSYSTEM
 * File: memory_core.c
 * ===================================================================== */

#include "memory_core.h"
#include <stdlib.h>
#include <string.h>

int mem_pool_init(MemoryPool *pool, long size)
{
    if (pool == NULL) return -1;
    pool->base = (char *)malloc((size_t)size);
    if (pool->base == NULL) {
        pool->size = 0;
        pool->used = 0;
        return -1;
    }
    pool->size = size;
    pool->used = 0;
    memset(pool->base, 0, (size_t)size);
    return 0;
}

void mem_pool_free(MemoryPool *pool)
{
    if (pool == NULL) return;
    if (pool->base != NULL) {
        free(pool->base);
        pool->base = NULL;
    }
    pool->size = 0;
    pool->used = 0;
}

void *mem_pool_alloc(MemoryPool *pool, long nbytes)
{
    char *ptr;
    long aligned_nbytes;

    if (pool == NULL || pool->base == NULL || nbytes <= 0) {
        return NULL;
    }

    /* Align to 8-byte boundary */
    aligned_nbytes = (nbytes + 7L) & ~7L;

    if (pool->used + aligned_nbytes > pool->size) {
        return NULL;  /* insufficient space */
    }

    ptr = pool->base + pool->used;
    pool->used += aligned_nbytes;
    return (void *)ptr;
}

void mem_pool_reset(MemoryPool *pool)
{
    if (pool != NULL) {
        pool->used = 0;
    }
}

long mem_pool_available(MemoryPool *pool)
{
    if (pool == NULL || pool->base == NULL) return 0;
    return pool->size - pool->used;
}
