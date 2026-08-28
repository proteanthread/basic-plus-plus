// FILENAME: gfx_primitives_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (gfx_draw_core.c, gfx_primitives.c, gfx_stmt_retro.c)
// NEEDED BY: libkernel (gfx_stmt_shapes.c)
// NEEDS: libcore (alloc.h, alloc.c, ctype.h, ctype.c, hal.h)
// NEEDS: libcore (memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (bgi.h, bgi.c, eval.h, eval.c, lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (gfx_internal.h)
// Implements virtual device and graphics rendering logic for gfx_primitives_internal.
//
// ---- Includes ----

#ifndef DEVICE_GFX_PRIMITIVES_INTERNAL_H
#define DEVICE_GFX_PRIMITIVES_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

#include "device/bgi.h"
#include "device/gfx_internal.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "vm/vm.h"


//
// ---- Coordinate Tracking ----

extern int g_last_plot_x;
extern int g_last_plot_y;

//
// ---- Drawing Algorithms ----

void gfx_pset(int x, int y, uint32_t col);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void draw_circle(int cx, int cy, int r, uint32_t color);

//
// ---- Statement Handlers ----

BppError vdev_legacy_stmt_line_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_circle_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_pset_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_preset_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_cls_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_paint_handler(VMContext *vm, LexerContext *lex);

BppError vdev_legacy_stmt_gr_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_hgr_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_hgr2_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_hcolor_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_plot_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_hlin_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_vlin_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_hplot_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_graphics_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_drawto_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_border_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_ink_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_paper_handler(VMContext *vm, LexerContext *lex);

#endif // DEVICE_GFX_PRIMITIVES_INTERNAL_H
