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
 *    - Simulated segmented RAM layout mapping for PEEK, POKE, DEF SEG, and VARPTR.
 *    - Emulation of x86 segmentation register bounds and DEF SEG offset routing.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#include "gw_memory.h"
#include <stdlib.h>
#include <string.h>

#define MAX_HOOKS 64

typedef struct {
    uint32_t start;
    uint32_t end;
    GW_MemReadHook hook;
    void *ctx;
} GW_ReadHookEntry;

typedef struct {
    uint32_t start;
    uint32_t end;
    GW_MemWriteHook hook;
    void *ctx;
} GW_WriteHookEntry;

struct GW_Memory {
    uint8_t *buffer;
    size_t size;
    uint16_t def_seg;
    
    GW_ReadHookEntry read_hooks[MAX_HOOKS];
    int read_hook_count;
    
    GW_WriteHookEntry write_hooks[MAX_HOOKS];
    int write_hook_count;
};

GW_Memory *gw_mem_create(size_t size) {
    GW_Memory *mem = (GW_Memory *)malloc(sizeof(GW_Memory));
    if (!mem) return NULL;
    
    mem->buffer = (uint8_t *)calloc(1, size);
    if (!mem->buffer) {
        free(mem);
        return NULL;
    }
    
    mem->size = size;
    mem->def_seg = 0;
    mem->read_hook_count = 0;
    mem->write_hook_count = 0;
    
    return mem;
}

void gw_mem_destroy(GW_Memory *mem) {
    if (mem) {
        free(mem->buffer);
        free(mem);
    }
}

void gw_mem_def_seg(GW_Memory *mem, uint16_t segment) {
    if (mem) {
        mem->def_seg = segment;
    }
}

uint32_t gw_mem_resolve(GW_Memory *mem, uint16_t offset) {
    if (!mem) return 0;
    return ((uint32_t)mem->def_seg * 16) + offset;
}

uint8_t gw_mem_peek(GW_Memory *mem, uint16_t offset) {
    if (!mem) return 0;
    
    uint32_t addr = gw_mem_resolve(mem, offset);
    
    // Check read hooks
    for (int i = 0; i < mem->read_hook_count; i++) {
        if (addr >= mem->read_hooks[i].start && addr <= mem->read_hooks[i].end) {
            return mem->read_hooks[i].hook(addr, mem->read_hooks[i].ctx);
        }
    }
    
    return mem->buffer[addr % mem->size];
}

void gw_mem_poke(GW_Memory *mem, uint16_t offset, uint8_t val) {
    if (!mem) return;
    
    uint32_t addr = gw_mem_resolve(mem, offset);
    
    // Check write hooks
    for (int i = 0; i < mem->write_hook_count; i++) {
        if (addr >= mem->write_hooks[i].start && addr <= mem->write_hooks[i].end) {
            mem->write_hooks[i].hook(addr, val, mem->write_hooks[i].ctx);
            return;
        }
    }
    
    mem->buffer[addr % mem->size] = val;
}

void gw_mem_register_read_hook(GW_Memory *mem, uint32_t start, uint32_t end, GW_MemReadHook hook, void *ctx) {
    if (!mem || mem->read_hook_count >= MAX_HOOKS) return;
    
    mem->read_hooks[mem->read_hook_count].start = start;
    mem->read_hooks[mem->read_hook_count].end = end;
    mem->read_hooks[mem->read_hook_count].hook = hook;
    mem->read_hooks[mem->read_hook_count].ctx = ctx;
    mem->read_hook_count++;
}

void gw_mem_register_write_hook(GW_Memory *mem, uint32_t start, uint32_t end, GW_MemWriteHook hook, void *ctx) {
    if (!mem || mem->write_hook_count >= MAX_HOOKS) return;
    
    mem->write_hooks[mem->write_hook_count].start = start;
    mem->write_hooks[mem->write_hook_count].end = end;
    mem->write_hooks[mem->write_hook_count].hook = hook;
    mem->write_hooks[mem->write_hook_count].ctx = ctx;
    mem->write_hook_count++;
}
