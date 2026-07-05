/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: graphics_core.h
 * Subsystem: Portable Software Rasterizer Engine
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Performs software rasterization (lines, circles, flood fills).
 *
 * 2. WHAT TO EXPECT:
 *    Renders geometric shapes directly into a screen buffer.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Resolution boundaries, clipping coordinates.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Bresenham line and circle algorithms.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If shapes draw offscreen, verify viewport bounds.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE SOFTWARE RASTERIZER ENGINE
 * File: graphics_core.h
 * ===================================================================== */

#ifndef STANDALONE_GRAPHICS_CORE_H
#define STANDALONE_GRAPHICS_CORE_H

#define GFX_CORE_WIDTH 320
#define GFX_CORE_HEIGHT 200

typedef struct GraphicsCanvas {
    unsigned char *pixels; /* Flat byte buffer of dimensions width * height */
    int width;
    int height;
    int clip_x1;
    int clip_y1;
    int clip_x2;
    int clip_y2;
} GraphicsCanvas;

/* Core drawing functions */
void graphics_core_clear(GraphicsCanvas *canvas, int color);
void graphics_core_pset(GraphicsCanvas *canvas, int x, int y, int color);
int graphics_core_point(const GraphicsCanvas *canvas, int x, int y);

/* Vector primitives */
void graphics_core_line(GraphicsCanvas *canvas, int x1, int y1, int x2, int y2, int color);
void graphics_core_box(GraphicsCanvas *canvas, int x1, int y1, int x2, int y2, int color, int filled);
void graphics_core_circle(GraphicsCanvas *canvas, int cx, int cy, int r, int color);

/* Flood Fill algorithm using a client-supplied stack buffer */
typedef struct { int x, y; } GraphicsCorePoint;
void graphics_core_paint(GraphicsCanvas *canvas, int x, int y, int fill_color, int border_color,
                          GraphicsCorePoint *stack, int stack_capacity);

/* ANSI TTY half-block renderer */
typedef void (*GraphicsCorePrintStrCallback)(void *user_data, const char *str);
typedef void (*GraphicsCorePrintCharCallback)(void *user_data, char c);

void graphics_core_render_ansi(const GraphicsCanvas *canvas,
                               const int *palette, int max_colors,
                               GraphicsCorePrintStrCallback print_str,
                               GraphicsCorePrintCharCallback print_char,
                               void *user_data);

#endif /* STANDALONE_GRAPHICS_CORE_H */
