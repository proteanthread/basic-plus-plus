/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_memory.h
 * @brief Memory context and allocation manager API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares memory pool contexts, arena operations, program line storage,
 *   and allocator hooks for strings and execution stacks.
 * - Why it exists: Decouples the interpreter from direct OS malloc/free calls during execution,
 *   preventing memory leaks, fragmentation, and platform-specific crash behaviors.
 * - Why it works this way: It provides structured arenas (scratch, program, string, variables)
 *   that scale dynamically on modern systems but fit conventional limits on FreeDOS.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Additional arena types or statistic tracking fields.
 * - What cannot be changed: Opaque pointer design (MemoryContext must remain private to mem_system.c).
 * - What to expect: Changes to allocations will impact the garbage collector and variable lifetimes.
 * - What to do if something breaks: Check memory limits, verify that mem_scratch_reset is called
 *   at the REPL command loop boundaries, and trace memory usage statistics.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Relies on host standard library malloc/free only during initialization (mem_init)
 *   and shutdown (mem_shutdown) phases.
 * - Portability concerns: Memory sizes must fit within the size_t limits of the target platform (16-bit vs 32/64-bit).
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add tracking and verification tags inside the private context.
 * - How to write external extensions: Extensions query memory stats or request scratch buffers via these APIs.
 */

#ifndef BPP_MEMORY_H
#define BPP_MEMORY_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"

/* Opaque Memory Context declaration */
typedef struct MemoryContext MemoryContext;

/* Structure representing a stored program line */
typedef struct {
    BppLineNumber line_number;
    char         *text;
} BppProgramLine;

/**
 * @brief Initialize the global memory system with specific pool sizes.
 * @return Opaque pointer to the initialized MemoryContext, or NULL on failure.
 */
MemoryContext *mem_init(size_t prog_mem_sz, size_t var_mem_sz, size_t str_mem_sz, size_t scratch_mem_sz);

/**
 * @brief Free all allocated memory blocks and shut down the memory manager.
 */
void mem_shutdown(MemoryContext *ctx);

/**
 * @brief Allocate memory from the ephemeral scratch arena.
 * @note Scratch memory is automatically reset after statement/command execution.
 */
void *mem_scratch_alloc(MemoryContext *ctx, size_t size);

/**
 * @brief Reset the scratch arena, invalidating all ephemeral allocations.
 */
void mem_scratch_reset(MemoryContext *ctx);

/**
 * @brief Insert or replace a BASIC program line.
 * @return true on success, false on OOM or duplicate.
 */
bool mem_program_insert(MemoryContext *ctx, BppLineNumber line, const char *text);

/**
 * @brief Delete a BASIC program line.
 * @return true if found and deleted, false otherwise.
 */
bool mem_program_delete(MemoryContext *ctx, BppLineNumber line);

/**
 * @brief Retrieve a BASIC program line's text.
 * @return Text pointer or NULL if line doesn't exist.
 */
const char *mem_program_get(MemoryContext *ctx, BppLineNumber line);

/**
 * @brief Retrieve all program lines for listing or compiling.
 * @param count Output pointer to receive line count.
 * @return Array of BppProgramLine structs sorted by line number.
 */
BppProgramLine *mem_program_get_all(MemoryContext *ctx, size_t *count);

/**
 * @brief Clear the program store (NEW command).
 */
void mem_program_clear(MemoryContext *ctx);

/**
 * @brief Insert a program line into the companion library program store.
 */
bool mem_lib_program_insert(MemoryContext *ctx, BppLineNumber line, const char *text);

/**
 * @brief Retrieve all companion library program lines.
 */
BppProgramLine *mem_lib_program_get_all(MemoryContext *ctx, size_t *count);

/**
 * @brief Clear the companion library program store.
 */
void mem_lib_program_clear(MemoryContext *ctx);

/**
 * @brief Allocator hook for the isolated string manager.
 */
void *mem_string_alloc(MemoryContext *ctx, size_t size);

/**
 * @brief Free hook for the isolated string manager.
 */
void mem_string_free(MemoryContext *ctx, void *ptr);

/**
 * @brief Retrieve total amount of free memory (in bytes) available to the user.
 */
size_t mem_get_free_ram(MemoryContext *ctx);

/**
 * @brief Retrieve total amount of used memory (in bytes).
 */
size_t mem_get_used_ram(MemoryContext *ctx);

/**
 * @brief Formats size in bytes to a human-readable string (e.g. MB, KB, Bytes).
 */
void mem_format_size(size_t bytes, char *buf, size_t buf_size);

#endif /* BPP_MEMORY_H */
