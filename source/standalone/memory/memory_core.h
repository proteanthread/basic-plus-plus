/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: memory_core.h
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
 * File: memory_core.h
 * =====================================================================
 * Standard C interface for generic bump-allocated memory pools.
 * Completely independent of the interpreter.
 * ===================================================================== */

#ifndef STANDALONE_MEMORY_CORE_H
#define STANDALONE_MEMORY_CORE_H

typedef struct MemoryPool {
    char *base;
    long size;
    long used;
} MemoryPool;

/* Initialize/Free a pool */
int mem_pool_init(MemoryPool *pool, long size);
void mem_pool_free(MemoryPool *pool);

/* Core allocation APIs */
void *mem_pool_alloc(MemoryPool *pool, long nbytes);
void mem_pool_reset(MemoryPool *pool);
long mem_pool_available(MemoryPool *pool);

#endif /* STANDALONE_MEMORY_CORE_H */
