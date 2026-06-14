/*
 * ---
 * BASIC++ Interpreter - builtins_io.c
 * ---
 *
 * Input/Output function handlers for the built-in function registry.
 *
 * Contains handlers for console and display state queries.
 *
 * ---
 */

#include "builtins.h"
#include "runtime.h"
#include "value.h"

/*
 * CSRLIN - Returns the current cursor row (1-based).
 * GW-BASIC: returns the vertical cursor position.
 * We track this via rt->cursor_row which is updated
 * by PRINT and LOCATE.
 * Category: FCAT_UTIL | Safety: FSAFE_STATE
 */
BValue builtin_csrlin(BValue *args, int argc, void *rt_ptr)
{
 RuntimeState *rs = (RuntimeState *)rt_ptr;
 (void)args; (void)argc;
 return bval_int((long)rs->cursor_row);
}
