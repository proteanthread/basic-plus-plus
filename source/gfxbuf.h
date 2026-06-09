/*
 * =====================================================================
 * BASIC++ Interpreter - gfxbuf.h
 * =====================================================================
 *
 * Virtual Graphics Framebuffer for terminal-based pixel graphics.
 *
 * PURPOSE:
 *   Provides PSET, LINE, CIRCLE, PAINT, PALETTE functionality
 *   in a pure terminal environment. Uses a virtual pixel buffer
 *   that renders to the terminal using ANSI escape codes and
 *   Unicode half-block characters (U+2580 upper, U+2584 lower).
 *
 *   Each terminal character cell represents 2 vertical pixels,
 *   giving effective resolution of 320x200 in an 80x100 grid.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_GFXBUF_H
#define BASICPP_GFXBUF_H

#include "config.h"

/* Initialize framebuffer (clear to color 0) */
void gfxbuf_init(void);

/* Clear framebuffer to a color */
void gfxbuf_clear(int color);

/* Set a pixel */
void gfxbuf_pset(int x, int y, int color);

/* Get a pixel color */
int  gfxbuf_point(int x, int y);

/* Draw a line (Bresenham) */
void gfxbuf_line(int x1, int y1, int x2, int y2, int color);

/* Draw a rectangle (B=outline, BF=filled) */
void gfxbuf_box(int x1, int y1, int x2, int y2,
                 int color, int filled);

/* Draw a circle (midpoint algorithm) */
void gfxbuf_circle(int cx, int cy, int r, int color);

/* Flood fill */
void gfxbuf_paint(int x, int y, int fill_color,
                   int border_color);

/* Remap palette entry */
void gfxbuf_palette(int attr, int color);

/* Render framebuffer to terminal */
void gfxbuf_render(void);

/* Check if graphics mode is active */
int  gfxbuf_active(void);

/* Set graphics mode on/off */
void gfxbuf_set_active(int on);

#endif /* BASICPP_GFXBUF_H */
