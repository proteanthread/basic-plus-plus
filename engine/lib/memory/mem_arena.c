// FILENAME: mem_arena.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c, string.h)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for mem_arena within BASIC++.
//
// ---- Includes ----

#include "memory/memory.h"
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
    // Align allocation to 8-byte boundary
    size_t aligned_size = (size + 7) & ~(size_t)7;
    if (arena->used + aligned_size > arena->capacity) {
        return NULL; // Out of arena memory
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
