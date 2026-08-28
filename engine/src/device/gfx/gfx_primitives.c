// FILENAME: gfx_primitives.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libkernel (gfx_primitives_internal.h)
// Implements virtual device and graphics rendering logic for gfx_primitives.
//
// ---- Includes ----

#include "device/gfx_primitives_internal.h"

// All graphics primitives subsystem logic is decomposed into:
// - device/gfx/primitives/gfx_draw_core.c: Bresenham lines, midpoint circles, and scanline flood fills
// - device/gfx/primitives/gfx_stmt_shapes.c: Geometric shape statement handlers (LINE, CIRCLE, PSET, PRESET, CLS)
// - device/gfx/primitives/gfx_stmt_retro.c: Vintage graphics statements (GR, HGR, PLOT, HLIN, VLIN, GRAPHICS, DRAWTO, INK, PAPER)
