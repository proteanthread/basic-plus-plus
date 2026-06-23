/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Interface definitions for virtual segmented memory mapping and memory storage.
 *    - Virtual segmented memory block accessors and segment definition registers.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#ifndef GW_MEMORY_H
#define GW_MEMORY_H

#include <stdint.h>
#include <stddef.h>

typedef struct GW_Memory GW_Memory;

// Read/Write Hooks to allow mapping external hardware or video memory
typedef uint8_t (*GW_MemReadHook)(uint32_t addr, void *ctx);
typedef void (*GW_MemWriteHook)(uint32_t addr, uint8_t val, void *ctx);

GW_Memory *gw_mem_create(size_t size);
void gw_mem_destroy(GW_Memory *mem);

// Segment Management
void gw_mem_def_seg(GW_Memory *mem, uint16_t segment);
uint32_t gw_mem_resolve(GW_Memory *mem, uint16_t offset); // returns segment*16 + offset

// Direct Operations
uint8_t gw_mem_peek(GW_Memory *mem, uint16_t offset);
void gw_mem_poke(GW_Memory *mem, uint16_t offset, uint8_t val);

// Register address range hooks
void gw_mem_register_read_hook(GW_Memory *mem, uint32_t start, uint32_t end, GW_MemReadHook hook, void *ctx);
void gw_mem_register_write_hook(GW_Memory *mem, uint32_t start, uint32_t end, GW_MemWriteHook hook, void *ctx);

#endif // GW_MEMORY_H
