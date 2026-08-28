// FILENAME: arena.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (alloc.h, alloc.c, arena.h, memops.h, memops.c)
// Freestanding static arena allocator implementation.
//
// ---- Includes ----

#include "runtime/memory/arena.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"

#define ARENA_DEFAULT_ALIGNMENT 16

bool runtime_arena_init(RuntimeArena *arena, void *buffer, size_t capacity) {
    if (!arena || !buffer || capacity == 0) {
        return false;
    }
    arena->buffer = (uint8_t *)buffer;
    arena->capacity = capacity;
    arena->offset = 0;
    arena->peak_offset = 0;
    arena->owns_buffer = false;
    return true;
}

RuntimeArena *runtime_arena_create(size_t capacity) {
    if (capacity == 0) {
        return NULL;
    }
    RuntimeArena *arena = (RuntimeArena *)runtime_malloc(sizeof(RuntimeArena));
    if (!arena) {
        return NULL;
    }
    uint8_t *buf = (uint8_t *)runtime_malloc(capacity);
    if (!buf) {
        runtime_free(arena);
        return NULL;
    }
    arena->buffer = buf;
    arena->capacity = capacity;
    arena->offset = 0;
    arena->peak_offset = 0;
    arena->owns_buffer = true;
    return arena;
}

void runtime_arena_destroy(RuntimeArena *arena) {
    if (!arena) {
        return;
    }
    if (arena->owns_buffer && arena->buffer) {
        runtime_free(arena->buffer);
    }
    runtime_free(arena);
}

void *runtime_arena_alloc(RuntimeArena *arena, size_t size, size_t align) {
    if (!arena || !arena->buffer || size == 0) {
        return NULL;
    }

    if (align == 0) {
        align = ARENA_DEFAULT_ALIGNMENT;
    }

    uintptr_t curr_addr = (uintptr_t)(arena->buffer + arena->offset);
    uintptr_t aligned_addr = (curr_addr + (align - 1)) & ~(align - 1);
    size_t padding = (size_t)(aligned_addr - curr_addr);

    if (arena->offset + padding + size > arena->capacity) {
        return NULL; // Out of arena capacity
    }

    arena->offset += padding + size;
    if (arena->offset > arena->peak_offset) {
        arena->peak_offset = arena->offset;
    }

    return (void *)aligned_addr;
}

void *runtime_arena_calloc(RuntimeArena *arena, size_t count, size_t size, size_t align) {
    size_t total = count * size;
    if (count != 0 && total / count != size) {
        return NULL; // Overflow
    }
    void *ptr = runtime_arena_alloc(arena, total, align);
    if (ptr) {
        runtime_memset(ptr, 0, total);
    }
    return ptr;
}

void runtime_arena_reset(RuntimeArena *arena) {
    if (arena) {
        arena->offset = 0;
    }
}

size_t runtime_arena_save(const RuntimeArena *arena) {
    return arena ? arena->offset : 0;
}

void runtime_arena_restore(RuntimeArena *arena, size_t checkpoint) {
    if (arena && checkpoint <= arena->capacity) {
        arena->offset = checkpoint;
    }
}
