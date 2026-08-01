/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * What it does: Implements a standalone memory arena allocator (scratch bump allocator and string heap).
 * Why it exists: Provides pure zero-VM memory allocation capabilities for standalone consumption.
 * Why it works this way: Uses a simple 8-byte aligned bump pointer pattern for O(1) allocation/reset.
 * What can be changed: Alignment size (default 8 bytes), scratch pool initial size.
 * What cannot be changed: 8-byte double alignment guarantee on 64-bit platforms.
 * What to expect: Blazing fast ephemeral scratch memory allocation and instant O(1) resets.
 * What to do if something breaks: Check alignment assertions and capacity limits.
 * Assumptions: Alignment requirements fit standard size_t arithmetic.
 * Portability concerns: Strict C17 compliant, pure 7-bit ASCII.
 * Future expansions: Add heap fragmentation metrics and custom pool hooks.
 * External extension hooks: Exposed via bpp_memory.h.
 */

#include "bpp_memory.h"
#include <stdlib.h>
#include <string.h>

struct MemArena {
    char   *base;
    size_t  capacity;
    size_t  used;
};

MemArena *mem_arena_create(size_t capacity) {
    if (capacity == 0) capacity = 64 * 1024;
    MemArena *arena = (MemArena *)calloc(1, sizeof(MemArena));
    if (!arena) return NULL;
    arena->base = (char *)calloc(1, capacity);
    if (!arena->base) {
        free(arena);
        return NULL;
    }
    arena->capacity = capacity;
    arena->used = 0;
    return arena;
}

void mem_arena_destroy(MemArena *arena) {
    if (!arena) return;
    if (arena->base) free(arena->base);
    free(arena);
}

void *mem_arena_alloc(MemArena *arena, size_t size) {
    if (!arena || !arena->base) return NULL;
    /* Align allocation to 8-byte boundary */
    size_t aligned_size = (size + 7) & ~(size_t)7;
    if (arena->used + aligned_size > arena->capacity) {
        return NULL; /* Out of arena memory */
    }
    void *ptr = arena->base + arena->used;
    arena->used += aligned_size;
    memset(ptr, 0, size);
    return ptr;
}

void mem_arena_reset(MemArena *arena) {
    if (!arena) return;
    arena->used = 0;
}

size_t mem_arena_get_used(const MemArena *arena) {
    return arena ? arena->used : 0;
}

size_t mem_arena_get_capacity(const MemArena *arena) {
    return arena ? arena->capacity : 0;
}
