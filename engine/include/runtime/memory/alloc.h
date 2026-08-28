// FILENAME: alloc.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel
// NEEDS: platform, memory
// Freestanding heap and dynamic memory allocation abstractions.
//
// ---- Includes ----

#ifndef RUNTIME_MEMORY_ALLOC_H
#define RUNTIME_MEMORY_ALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Memory statistics descriptor
typedef struct {
    size_t total_pool_size;
    size_t bytes_allocated;
    size_t bytes_free;
    size_t peak_allocated;
    size_t num_allocations;
    size_t num_frees;
    size_t active_blocks;
} RuntimeMemStats;

// Pluggable memory hooks for runtime
typedef struct {
    void *(*malloc)(size_t size);
    void *(*calloc)(size_t count, size_t size);
    void *(*realloc)(void *ptr, size_t size);
    void  (*free)(void *ptr);
    void  (*lock)(void);
    void  (*unlock)(void);
} RuntimeMemHooks;

// @brief Initialize the global freestanding memory pool from a raw buffer.
bool runtime_mem_init_pool(void *buffer, size_t pool_size);

// @brief Configure custom external memory hooks (e.g., host libc or OS allocator).
void runtime_mem_set_hooks(const RuntimeMemHooks *hooks);

// @brief Retrieve the active memory hooks.
const RuntimeMemHooks *runtime_mem_get_hooks(void);

// @brief Primary allocation function.
void *runtime_malloc(size_t size);

// @brief Primary zero-initialized allocation function.
void *runtime_calloc(size_t count, size_t size);

// @brief Primary reallocation function.
void *runtime_realloc(void *ptr, size_t size);

// @brief Primary deallocation function.
void runtime_free(void *ptr);

// @brief Query runtime memory statistics.
void runtime_mem_get_stats(RuntimeMemStats *out_stats);

// @brief Reset memory statistics.
void runtime_mem_reset_stats(void);

// @brief Check integrity of the freestanding memory pool (returns true if healthy).
bool runtime_mem_verify_integrity(void);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_MEMORY_ALLOC_H
