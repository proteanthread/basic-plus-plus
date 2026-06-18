 // ---
 // BASIC++ Interpreter - builtins_graphics.c
 // ---
 //
 // Graphics function handlers for the built-in function registry.
 //
 // Contains handlers for pixel queries and graphics buffer access.
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
#include "gfxbuf.h"
#include "value.h"

 // POINT(x, y) - Read the color of a pixel from the graphics buffer.
 // With 1 arg: POINT(x) returns current cursor position.
 // With 2 args: POINT(x, y) returns pixel color.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
BValue builtin_point(BValue *args, int argc, void *rt)
{
 (void)rt;
 if (argc >= 2) {
 int x = (int)bval_to_int(&args[0]);
 int y = (int)bval_to_int(&args[1]);
 return bval_int((long)gfxbuf_point(x, y));
 }
 return bval_int(0);
}
