/*
 * ---
 * BASIC++ Interpreter - builtins_graphics.c
 * ---
 *
 * Graphics function handlers for the built-in function registry.
 *
 * Contains handlers for pixel queries and graphics buffer access.
 *
 * ---
 */

#include "builtins.h"
#include "gfxbuf.h"
#include "value.h"

/*
 * POINT(x, y) - Read the color of a pixel from the graphics buffer.
 * With 1 arg: POINT(x) returns current cursor position.
 * With 2 args: POINT(x, y) returns pixel color.
 * Category: FCAT_UTIL | Safety: FSAFE_STATE
 */
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
