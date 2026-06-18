 // ---
 // BASIC++ Interpreter - builtins_memory.c
 // ---
 //
 // Memory function handlers for the built-in function registry.
 //
 // Contains handlers for virtual memory access and memory
 // size queries.
 //
//
// HOW TO EXTEND:
//   To add a new built-in function:
//   1. Write a handler: BValue my_func(BValue *args, int argc, void *ctx)
//   2. Register it in the init function with funcreg_add().
//   3. Specify min/max argument counts and return type.
//
// TROUBLESHOOTING:
//   - Wrong arg count: check min_args/max_args in registration.
//   - Type mismatch: use bval_to_float/bval_to_int for conversion.
 // ---

#include "builtins.h"
#include "runtime.h"
#include "config.h"
#include "value.h"

 // PEEK(address) - Read a byte from the virtual memory segment.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
BValue builtin_peek(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 long addr;
 int offset;
 (void)argc;

 addr = bval_to_int(&args[0]);
 offset = state->mem_seg_base + (int)addr;
 if (offset < 0 || offset >= MAX_MEM_SEGMENT)
 return bval_int(0);
 return bval_int((long)state->mem_segment[offset]);
}

 // SIZE - Available memory.
 //
 // Returns the number of available bytes in the program memory pool.
 // Takes no arguments (argc=0).
 //
 // Category: FCAT_UTIL | Safety: FSAFE_PURE
BValue builtin_size(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)args; (void)argc;
 return bval_int(runtime_size(state));
}
