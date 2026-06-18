 // ---
 // BASIC++ Interpreter - builtins_io.c
 // ---
 //
 // Input/Output function handlers for the built-in function registry.
 //
 // Contains handlers for console and display state queries.
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
#include "value.h"

 // CSRLIN - Returns the current cursor row (1-based).
 // GW-BASIC: returns the vertical cursor position.
 // We track this via rt->cursor_row which is updated
 // by PRINT and LOCATE.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
BValue builtin_csrlin(BValue *args, int argc, void *rt_ptr)
{
 RuntimeState *rs = (RuntimeState *)rt_ptr;
 (void)args; (void)argc;
 return bval_int((long)rs->cursor_row);
}
