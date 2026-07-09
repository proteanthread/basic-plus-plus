/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: builtins_graphics.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Screen mode setting, line/circle drawing algorithms, palette mappings, and ANSI/Unicode text-mode framebuffer.
 *
 * 2. WHAT TO EXPECT:
 *    Renders coordinate-scaled virtual buffer to terminal using Unicode half-blocks. High execution density.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Default palette colors, screen dimensions limits, drawing layout algorithms.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Framebuffer structure, Unicode screen printing characters mappings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If display is garbled, verify terminal supports UTF-8 and ANSI escape codes. Check coordinate math ranges.
 * ===================================================================== */

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
#include "../sdl2_emu.h"
#include "../runtime.h"

 // POINT(x, y) - Read the color of a pixel from the graphics buffer.
 // With 1 arg: POINT(x) returns current cursor position.
 // With 2 args: POINT(x, y) returns pixel color.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
BValue builtin_point(BValue *args, int argc, void *rt_ptr)
{
    RuntimeState *rt = (RuntimeState *)rt_ptr; (void)rt;
    if (argc >= 2) {
        int x = (int)bval_to_int(&args[0]);
        int y = (int)bval_to_int(&args[1]);
        return bval_int((long)gfxbuf_point(x, y));
    } else if (argc == 1) {
        int n = (int)bval_to_int(&args[0]);
        int cx = 0, cy = 0;
        gfxbuf_get_cursor(&cx, &cy);
        if (n == 0) return bval_int(cx); // current X
        if (n == 1) return bval_int(cy); // current Y
        if (n == 2) return bval_int(0);  // world X (not implemented)
        if (n == 3) return bval_int(0);  // world Y
    }
    return bval_int(0);
}


// PEN(n) - Light pen / Mouse proxy
BValue builtin_pen(BValue *args, int argc, void *rt_ptr)
{
    RuntimeState *rt = (RuntimeState *)rt_ptr; (void)rt;
    int n = 0;
    if (argc > 0) {
        n = (int)bval_to_int(&args[0]);
    }
    
    int mx = 0, my = 0, mdown = 0, mwas_down = 0, mlast_x = 0, mlast_y = 0;
    gw_sdl2_get_pen(&mx, &my, &mdown, &mwas_down, &mlast_x, &mlast_y);

    if (n == 0) return bval_int(mwas_down);
    if (n == 1) return bval_int(mlast_x);
    if (n == 2) return bval_int(mlast_y);
    if (n == 3) return bval_int(mdown);
    if (n == 4) return bval_int(mx);
    if (n == 5) return bval_int(my);
    
    return bval_int(0);
}

// PMAP(n) - Coordinate translation
BValue builtin_pmap(BValue *args, int argc, void *rt_ptr)
{
    RuntimeState *rt = (RuntimeState *)rt_ptr; (void)rt;
    if (argc < 2) return bval_int(0);
    double expr = bval_to_float(&args[0]);
    int mode = (int)bval_to_int(&args[1]);
    
    if (!rt->win_active) {
        return bval_int((long)expr);
    }
    
    int w = gw_sdl2_get_width();
    int h = gw_sdl2_get_height();
    if (w <= 0) w = 640;
    if (h <= 0) h = 400;

    double p_width = (double)(w - 1);
    double p_height = (double)(h - 1);
    double l_width = rt->win_x2 - rt->win_x1;
    double l_height = rt->win_y2 - rt->win_y1;
    if (l_width == 0.0) l_width = 1.0;
    if (l_height == 0.0) l_height = 1.0;

    if (mode == 0) {
        double px = (expr - rt->win_x1) * p_width / l_width;
        return bval_int((long)px);
    } else if (mode == 1) {
        double py;
        if (rt->win_screen_flag) {
            py = (expr - rt->win_y1) * p_height / l_height;
        } else {
            py = p_height - ((expr - rt->win_y1) * p_height / l_height);
        }
        return bval_int((long)py);
    } else if (mode == 2) {
        double lx = rt->win_x1 + (expr * l_width / p_width);
        return bval_float(lx);
    } else if (mode == 3) {
        double ly;
        if (rt->win_screen_flag) {
            ly = rt->win_y1 + (expr * l_height / p_height);
        } else {
            ly = rt->win_y1 + ((p_height - expr) * l_height / p_height);
        }
        return bval_float(ly);
    }
    
    return bval_int(0);
}
