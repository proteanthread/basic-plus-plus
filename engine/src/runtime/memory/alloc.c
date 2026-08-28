// FILENAME: alloc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel
// NEEDS: libcore (alloc.h, memops.h, memops.c)
// Freestanding allocator bridge implementation.
//
// ---- Includes ----

#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"

// Boundary tag block header for freestanding pool
#define RUNTIME_MEM_MAGIC_ALLOC 0x5AA5BEEF
#define RUNTIME_MEM_MAGIC_FREE  0x5AA5DEAD
#define RUNTIME_MEM_ALIGNMENT   16

typedef struct RuntimeMemBlockHeader {
    uint32_t magic;
    uint32_t is_free;
    size_t size;                          // Payload size (excluding header & footer)
    struct RuntimeMemBlockHeader *prev;   // In physical address order
    struct RuntimeMemBlockHeader *next;
} RuntimeMemBlockHeader;

typedef struct RuntimeMemBlockFooter {
    uint32_t magic;
    size_t size;
} RuntimeMemBlockFooter;

#define BLOCK_HEADER_SIZE ((sizeof(RuntimeMemBlockHeader) + (RUNTIME_MEM_ALIGNMENT - 1)) & ~(RUNTIME_MEM_ALIGNMENT - 1))
#define BLOCK_FOOTER_SIZE ((sizeof(RuntimeMemBlockFooter) + (RUNTIME_MEM_ALIGNMENT - 1)) & ~(RUNTIME_MEM_ALIGNMENT - 1))
#define MIN_BLOCK_SIZE    (BLOCK_HEADER_SIZE + BLOCK_FOOTER_SIZE + RUNTIME_MEM_ALIGNMENT)

// Static default pool for freestanding/bare-metal environments (64 MB default if uninitialized)
#ifndef RUNTIME_STATIC_POOL_SIZE
#define RUNTIME_STATIC_POOL_SIZE (64 * 1024 * 1024)
#endif

// State
static RuntimeMemHooks active_hooks = {0};
static uint8_t *pool_start = NULL;
static size_t pool_total_size = 0;
static RuntimeMemBlockHeader *first_block = NULL;
static RuntimeMemStats mem_stats = {0};

static inline void mem_lock(void) {
    if (active_hooks.lock) {
        active_hooks.lock();
    }
}

static inline void mem_unlock(void) {
    if (active_hooks.unlock) {
        active_hooks.unlock();
    }
}

static inline size_t align_size(size_t size) {
    return (size + (RUNTIME_MEM_ALIGNMENT - 1)) & ~(RUNTIME_MEM_ALIGNMENT - 1);
}

static inline RuntimeMemBlockFooter *get_footer(RuntimeMemBlockHeader *header) {
    uint8_t *payload = (uint8_t *)header + BLOCK_HEADER_SIZE;
    return (RuntimeMemBlockFooter *)(payload + header->size);
}

bool runtime_mem_init_pool(void *buffer, size_t pool_size) {
    if (!buffer || pool_size < (MIN_BLOCK_SIZE * 2)) {
        return false;
    }

    mem_lock();

    // Align start pointer
    uintptr_t raw = (uintptr_t)buffer;
    uintptr_t aligned = (raw + (RUNTIME_MEM_ALIGNMENT - 1)) & ~(RUNTIME_MEM_ALIGNMENT - 1);
    size_t offset = (size_t)(aligned - raw);
    if (offset >= pool_size) {
        mem_unlock();
        return false;
    }

    pool_start = (uint8_t *)aligned;
    pool_total_size = pool_size - offset;

    // Single initial free block
    first_block = (RuntimeMemBlockHeader *)pool_start;
    first_block->magic = RUNTIME_MEM_MAGIC_FREE;
    first_block->is_free = 1;
    first_block->size = pool_total_size - BLOCK_HEADER_SIZE - BLOCK_FOOTER_SIZE;
    first_block->prev = NULL;
    first_block->next = NULL;

    RuntimeMemBlockFooter *footer = get_footer(first_block);
    footer->magic = RUNTIME_MEM_MAGIC_FREE;
    footer->size = first_block->size;

    mem_stats.total_pool_size = pool_total_size;
    mem_stats.bytes_allocated = 0;
    mem_stats.bytes_free = first_block->size;
    mem_stats.peak_allocated = 0;
    mem_stats.num_allocations = 0;
    mem_stats.num_frees = 0;
    mem_stats.active_blocks = 0;

    mem_unlock();
    return true;
}

void runtime_mem_set_hooks(const RuntimeMemHooks *hooks) {
    mem_lock();
    if (hooks) {
        active_hooks = *hooks;
    } else {
        runtime_memset(&active_hooks, 0, sizeof(active_hooks));
    }
    mem_unlock();
}

const RuntimeMemHooks *runtime_mem_get_hooks(void) {
    return &active_hooks;
}

// Built-in pool allocation routine (best-fit with splitting)
static void *pool_alloc(size_t size) {
    if (!pool_start) {
        // Fallback: lazily declare static pool if uninitialized
        static uint8_t default_static_pool[1024 * 1024]; // 1MB minimum static pool
        if (!runtime_mem_init_pool(default_static_pool, sizeof(default_static_pool))) {
            return NULL;
        }
    }

    size_t needed = align_size(size);
    RuntimeMemBlockHeader *best = NULL;
    size_t best_size = (size_t)-1;

    // Best-fit search
    RuntimeMemBlockHeader *curr = first_block;
    while (curr) {
        if (curr->is_free && curr->size >= needed) {
            if (curr->size < best_size) {
                best = curr;
                best_size = curr->size;
                if (best_size == needed) {
                    break; // Exact match
                }
            }
        }
        curr = curr->next;
    }

    if (!best) {
        return NULL; // Out of memory
    }

    // Split block if excess is large enough
    size_t excess = best->size - needed;
    if (excess >= MIN_BLOCK_SIZE) {
        size_t new_free_size = excess - BLOCK_HEADER_SIZE - BLOCK_FOOTER_SIZE;
        best->size = needed;

        RuntimeMemBlockFooter *best_footer = get_footer(best);
        best_footer->magic = RUNTIME_MEM_MAGIC_ALLOC;
        best_footer->size = needed;

        uint8_t *next_addr = (uint8_t *)best_footer + BLOCK_FOOTER_SIZE;
        RuntimeMemBlockHeader *split = (RuntimeMemBlockHeader *)next_addr;
        split->magic = RUNTIME_MEM_MAGIC_FREE;
        split->is_free = 1;
        split->size = new_free_size;
        split->prev = best;
        split->next = best->next;
        if (best->next) {
            best->next->prev = split;
        }
        best->next = split;

        RuntimeMemBlockFooter *split_footer = get_footer(split);
        split_footer->magic = RUNTIME_MEM_MAGIC_FREE;
        split_footer->size = new_free_size;
    }

    best->is_free = 0;
    best->magic = RUNTIME_MEM_MAGIC_ALLOC;
    RuntimeMemBlockFooter *footer = get_footer(best);
    footer->magic = RUNTIME_MEM_MAGIC_ALLOC;

    mem_stats.bytes_allocated += best->size;
    if (mem_stats.bytes_free >= best->size) {
        mem_stats.bytes_free -= best->size;
    } else {
        mem_stats.bytes_free = 0;
    }
    if (mem_stats.bytes_allocated > mem_stats.peak_allocated) {
        mem_stats.peak_allocated = mem_stats.bytes_allocated;
    }
    mem_stats.num_allocations++;
    mem_stats.active_blocks++;

    return (void *)((uint8_t *)best + BLOCK_HEADER_SIZE);
}

static void pool_free(void *ptr) {
    if (!ptr || !pool_start) {
        return;
    }

    RuntimeMemBlockHeader *block = (RuntimeMemBlockHeader *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
    if (block->magic != RUNTIME_MEM_MAGIC_ALLOC || block->is_free != 0) {
        return; // Double free or invalid pointer
    }

    block->is_free = 1;
    block->magic = RUNTIME_MEM_MAGIC_FREE;
    RuntimeMemBlockFooter *footer = get_footer(block);
    footer->magic = RUNTIME_MEM_MAGIC_FREE;

    mem_stats.bytes_allocated -= block->size;
    mem_stats.bytes_free += block->size;
    mem_stats.num_frees++;
    if (mem_stats.active_blocks > 0) {
        mem_stats.active_blocks--;
    }

    // Coalesce with next block if free
    if (block->next && block->next->is_free) {
        RuntimeMemBlockHeader *next = block->next;
        block->size += BLOCK_HEADER_SIZE + next->size + BLOCK_FOOTER_SIZE;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
        footer = get_footer(block);
        footer->magic = RUNTIME_MEM_MAGIC_FREE;
        footer->size = block->size;
    }

    // Coalesce with prev block if free
    if (block->prev && block->prev->is_free) {
        RuntimeMemBlockHeader *prev = block->prev;
        prev->size += BLOCK_HEADER_SIZE + block->size + BLOCK_FOOTER_SIZE;
        prev->next = block->next;
        if (block->next) {
            block->next->prev = prev;
        }
        footer = get_footer(prev);
        footer->magic = RUNTIME_MEM_MAGIC_FREE;
        footer->size = prev->size;
    }
}

void *runtime_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    mem_lock();
    void *ptr;
    if (active_hooks.malloc) {
        ptr = active_hooks.malloc(size);
        if (ptr) {
            mem_stats.bytes_allocated += size;
            if (mem_stats.bytes_allocated > mem_stats.peak_allocated) {
                mem_stats.peak_allocated = mem_stats.bytes_allocated;
            }
            mem_stats.num_allocations++;
            mem_stats.active_blocks++;
        }
    } else {
        ptr = pool_alloc(size);
    }
    mem_unlock();
    return ptr;
}

void *runtime_calloc(size_t count, size_t size) {
    size_t total = count * size;
    if (count != 0 && total / count != size) {
        return NULL; // Overflow
    }
    if (total == 0) {
        return NULL;
    }

    mem_lock();
    void *ptr;
    if (active_hooks.calloc) {
        ptr = active_hooks.calloc(count, size);
        if (ptr) {
            mem_stats.bytes_allocated += total;
            if (mem_stats.bytes_allocated > mem_stats.peak_allocated) {
                mem_stats.peak_allocated = mem_stats.bytes_allocated;
            }
            mem_stats.num_allocations++;
            mem_stats.active_blocks++;
        }
    } else {
        ptr = pool_alloc(total);
        if (ptr) {
            runtime_memset(ptr, 0, total);
        }
    }
    mem_unlock();
    return ptr;
}

void *runtime_realloc(void *ptr, size_t size) {
    if (!ptr) {
        return runtime_malloc(size);
    }
    if (size == 0) {
        runtime_free(ptr);
        return NULL;
    }

    mem_lock();
    void *new_ptr = NULL;
    if (active_hooks.realloc) {
        new_ptr = active_hooks.realloc(ptr, size);
    } else {
        RuntimeMemBlockHeader *block = (RuntimeMemBlockHeader *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
        if (block->magic == RUNTIME_MEM_MAGIC_ALLOC) {
            if (block->size >= size) {
                new_ptr = ptr; // Already big enough
            } else {
                new_ptr = pool_alloc(size);
                if (new_ptr) {
                    runtime_memcpy(new_ptr, ptr, block->size);
                    pool_free(ptr);
                }
            }
        }
    }
    mem_unlock();
    return new_ptr;
}

void runtime_free(void *ptr) {
    if (!ptr) {
        return;
    }
    mem_lock();
    if (active_hooks.free) {
        active_hooks.free(ptr);
        mem_stats.num_frees++;
        if (mem_stats.active_blocks > 0) {
            mem_stats.active_blocks--;
        }
    } else {
        pool_free(ptr);
    }
    mem_unlock();
}

void runtime_mem_get_stats(RuntimeMemStats *out_stats) {
    if (out_stats) {
        mem_lock();
        *out_stats = mem_stats;
        mem_unlock();
    }
}

void runtime_mem_reset_stats(void) {
    mem_lock();
    mem_stats.peak_allocated = mem_stats.bytes_allocated;
    mem_stats.num_allocations = mem_stats.active_blocks;
    mem_stats.num_frees = 0;
    mem_unlock();
}

bool runtime_mem_verify_integrity(void) {
    if (!pool_start) {
        return true;
    }
    mem_lock();
    RuntimeMemBlockHeader *curr = first_block;
    while (curr) {
        if (curr->magic != RUNTIME_MEM_MAGIC_ALLOC && curr->magic != RUNTIME_MEM_MAGIC_FREE) {
            mem_unlock();
            return false;
        }
        RuntimeMemBlockFooter *footer = get_footer(curr);
        if (footer->magic != curr->magic || footer->size != curr->size) {
            mem_unlock();
            return false;
        }
        curr = curr->next;
    }
    mem_unlock();
    return true;
}
