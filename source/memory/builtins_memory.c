/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: builtins_memory.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Utility or helper code for BASIC++ interpreter.
 *
 * 2. WHAT TO EXPECT:
 *    Executes with low overhead, relying on fixed compile-time limits and memory pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Internal helper functions, optimization passes, or local naming adjustments.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Public API structures, parameter contracts, or global type definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - builtins_memory.c
// ---
//
// Memory function handlers for the built-in function registry.
//
// Contains handlers for virtual memory access and memory
// size queries.

#include "builtins.h"
#include "runtime.h"
#include "config.h"
#include "value.h"

#ifdef BPP_LITE_BUILD
BValue builtin_peek(BValue *args, int argc, void *rt)
{
    (void)args; (void)argc; (void)rt;
    return bval_int(0);
}

BValue builtin_peekb(BValue *args, int argc, void *rt)
{
    (void)args; (void)argc; (void)rt;
    return bval_int(0);
}
#else
#include "gw_memory.h"
extern struct GW_Memory *g_gw_mem;

 // PEEK(address) - Read a byte from the virtual memory segment.
BValue builtin_peek(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 long addr;
 int offset;
 (void)argc;

 addr = bval_to_int(&args[0]);
 if (g_gw_mem != NULL) {
     return bval_int((long)gw_mem_peek(g_gw_mem, (uint32_t)addr));
 }
 offset = state->mem_seg_base + (int)addr;
 if (offset < 0 || offset >= MAX_MEM_SEGMENT)
 return bval_int(0);
 return bval_int((long)state->mem_segment[offset]);
}

// PEEKB(bank, offset) - Read a byte from a virtual memory RAMBANK.
BValue builtin_peekb(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    int bank;
    long offset;
    (void)argc;

    bank = (int)bval_to_int(&args[0]);
    offset = bval_to_int(&args[1]);

    return bval_int((long)rambank_peek(state->memory, bank, offset, 0));
}
#endif

 // SIZE - Available memory.
BValue builtin_size(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)args; (void)argc;
 return bval_int(runtime_size(state));
}
