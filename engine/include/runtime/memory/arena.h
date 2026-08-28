// FILENAME: arena.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (arena.c)
// NEEDS: platform, memory
// Freestanding monotonic arena allocator for static and embedded targets.
//
// ---- Includes ----

#ifndef RUNTIME_MEMORY_ARENA_H
#define RUNTIME_MEMORY_ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RuntimeArena {
    uint8_t *buffer;
    size_t capacity;
    size_t offset;
    size_t peak_offset;
    bool owns_buffer;
} RuntimeArena;

// @brief Initialize an arena from a user-supplied static or heap memory buffer.
bool runtime_arena_init(RuntimeArena *arena, void *buffer, size_t capacity);

// @brief Allocate a new arena using runtime_malloc.
RuntimeArena *runtime_arena_create(size_t capacity);

// @brief Destroy an arena allocated via runtime_arena_create.
void runtime_arena_destroy(RuntimeArena *arena);

// @brief Allocate size bytes with requested alignment (default alignment if align == 0).
void *runtime_arena_alloc(RuntimeArena *arena, size_t size, size_t align);

// @brief Allocate zero-initialized memory from the arena.
void *runtime_arena_calloc(RuntimeArena *arena, size_t count, size_t size, size_t align);

// @brief Reset the arena offset to zero (instant bulk deallocation).
void runtime_arena_reset(RuntimeArena *arena);

// @brief Save the current allocation offset (checkpoint).
size_t runtime_arena_save(const RuntimeArena *arena);

// @brief Rollback the arena offset to a previously saved checkpoint.
void runtime_arena_restore(RuntimeArena *arena, size_t checkpoint);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_MEMORY_ARENA_H
