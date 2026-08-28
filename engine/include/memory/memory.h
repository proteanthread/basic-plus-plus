// FILENAME: memory.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libserver
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for memory within BASIC++.
//
// ---- Includes ----

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"

// Opaque Memory Context declaration
typedef struct MemoryContext MemoryContext;

// Structure representing a stored program line
typedef struct {
    BppLineNumber line_number;
    char         *text;
    void         *ast_cache;
    bool          ast_valid;
    size_t        ast_skip_lines;
} BppProgramLine;

// @brief Initialize the global memory system with specific pool sizes.
// @return Opaque pointer to the initialized MemoryContext, or NULL on failure.
MemoryContext *mem_init(size_t prog_mem_sz, size_t var_mem_sz, size_t str_mem_sz, size_t scratch_mem_sz);

// @brief Free all allocated memory blocks and shut down the memory manager.
void mem_shutdown(MemoryContext *ctx);

// @brief Allocate memory from the ephemeral scratch arena.
// @note Scratch memory is automatically reset after statement/command execution.
void *mem_scratch_alloc(MemoryContext *ctx, size_t size);

// @brief Reset the scratch arena, invalidating all ephemeral allocations.
void mem_scratch_reset(MemoryContext *ctx);

// @brief Insert or replace a BASIC program line.
// @return true on success, false on OOM or duplicate.
bool mem_program_insert(MemoryContext *ctx, BppLineNumber line, const char *text);

// @brief Delete a BASIC program line.
// @return true if found and deleted, false otherwise.
bool mem_program_delete(MemoryContext *ctx, BppLineNumber line);

// Friendly aliases
#define mem_program_store(ctx, line, text) mem_program_insert((ctx), (line), (text))
#define mem_program_remove(ctx, line)      mem_program_delete((ctx), (line))

// @brief Retrieve a BASIC program line's text.
// @return Text pointer or NULL if line doesn't exist.
const char *mem_program_get(MemoryContext *ctx, BppLineNumber line);

// @brief Retrieve all program lines for listing or compiling.
// @param count Output pointer to receive line count.
// @return Array of BppProgramLine structs sorted by line number.
BppProgramLine *mem_program_get_all(MemoryContext *ctx, size_t *count);

// @brief Binary search to find the index of a program line.
// @param ctx Memory context.
// @param line Line number to find.
// @param out_idx Output pointer for the found index (or insertion index if not found).
// @return true if exact line was found, false otherwise.
bool mem_program_find_line_index(MemoryContext *ctx, BppLineNumber line, size_t *out_idx);

// @brief Check if stored program contains namespace statements.
bool mem_program_has_namespaces(MemoryContext *ctx);

// @brief Set whether stored program contains namespace statements.
void mem_program_set_has_namespaces(MemoryContext *ctx, bool has_ns);

// @brief Clear the program store (NEW command).
void mem_program_clear(MemoryContext *ctx);

// @brief Set the active program version string.
void mem_program_set_version(MemoryContext *ctx, const char *ver_str);

// @brief Get the active program version string (or empty string if untagged).
const char *mem_program_get_version(MemoryContext *ctx);

// @brief Insert a program line into the companion library program store.
bool mem_lib_program_insert(MemoryContext *ctx, BppLineNumber line, const char *text);

// @brief Retrieve all companion library program lines.
BppProgramLine *mem_lib_program_get_all(MemoryContext *ctx, size_t *count);

// @brief Clear the companion library program store.
void mem_lib_program_clear(MemoryContext *ctx);

// @brief Allocator hook for the isolated string manager.
void *mem_string_alloc(MemoryContext *ctx, size_t size);

// @brief Free hook for the isolated string manager.
void mem_string_free(MemoryContext *ctx, void *ptr);

// @brief Retrieve total amount of free memory (in bytes) available to the user.
size_t mem_get_free_ram(MemoryContext *ctx);

// @brief Retrieve total amount of used memory (in bytes).
size_t mem_get_used_ram(MemoryContext *ctx);

// @brief Formats size in bytes to a human-readable string (e.g. MB, KB, Bytes).
void mem_format_size(size_t bytes, char *buf, size_t buf_size);

#endif // MEMORY_H
