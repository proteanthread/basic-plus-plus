# BGI Graphics Interface API Reference

Header File: [`include/bgi.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bgi.h)

## Overview
Provides Borland Graphics Interface retro graphics mode, shapes, and drawing primitives.

## Exposed API Entities
### Structs & Types
- `BGI_VideoMode`
- `BGI_Context`

### Enums
- `BGI_MemLayout`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `BGI_init` | `int` | `BGI_Context *ctx` |
| `BGI_shutdown` | `void` | `BGI_Context *ctx` |
| `BGI_register_mode` | `int` | `BGI_Context *ctx, const BGI_VideoMode *mode` |
| `BGI_set_mode` | `int` | `BGI_Context *ctx, int mode_handle` |
| `BGI_set_mode_by_id` | `int` | `BGI_Context *ctx, uint32_t mode_id` |
| `BGI_get_dimensions` | `void` | `const BGI_Context *ctx, int *w, int *h` |
| `BGI_putpixel` | `void` | `BGI_Context *ctx, int x, int y, int color` |
| `BGI_getpixel` | `int` | `const BGI_Context *ctx, int x, int y` |
| `BGI_setcolor` | `void` | `BGI_Context *ctx, int color` |
| `BGI_setbkcolor` | `void` | `BGI_Context *ctx, int color` |
| `BGI_setfillstyle` | `void` | `BGI_Context *ctx, int style, int color` |
| `BGI_setlinestyle` | `void` | `BGI_Context *ctx, int style, int pattern, int thickness` |
| `BGI_line` | `void` | `BGI_Context *ctx, int x1, int y1, int x2, int y2` |
| `BGI_circle` | `void` | `BGI_Context *ctx, int cx, int cy, int r` |
| `BGI_fillellipse` | `void` | `BGI_Context *ctx, int cx, int cy, int rx, int ry` |
| `BGI_bar` | `void` | `BGI_Context *ctx, int x1, int y1, int x2, int y2` |
| `BGI_rectangle` | `void` | `BGI_Context *ctx, int x1, int y1, int x2, int y2` |
| `BGI_floodfill` | `void` | `BGI_Context *ctx, int seed_x, int seed_y, int border` |
| `BGI_moveto` | `void` | `BGI_Context *ctx, int x, int y` |
| `BGI_lineto` | `void` | `BGI_Context *ctx, int x, int y` |
| `BGI_clearviewport` | `void` | `BGI_Context *ctx` |
| `BGI_cleardevice` | `void` | `BGI_Context *ctx` |
| `BGI_setpalette` | `void` | `BGI_Context *ctx, int index, uint32_t argb` |
| `BGI_getpalette` | `uint32_t` | `const BGI_Context *ctx, int index` |
| `BGI_setallpalette` | `void` | `BGI_Context *ctx, const uint32_t *pal, int count` |
| `BGI_resolve_color` | `uint32_t` | `const BGI_Context *ctx, int color` |
| `BGI_outtextxy` | `void` | `BGI_Context *ctx, int x, int y, const char *text` |
| `BGI_settextstyle` | `void` | `BGI_Context *ctx, int font, int direction, int size` |
| `BGI_textwidth` | `int` | `const BGI_Context *ctx, const char *text` |
| `BGI_textheight` | `int` | `const BGI_Context *ctx, const char *text` |
| `BGI_imagesize` | `size_t` | `int x1, int y1, int x2, int y2` |
| `BGI_putimage` | `void` | `BGI_Context *ctx, int x, int y, const void *buffer, int op` |
| `BGI_synthesize` | `void` | `BGI_Context *ctx` |
| `BGI_register_heritage_modes` | `void` | `BGI_Context *ctx` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bgi.h"

void draw_line() {
    BGI_line(NULL, 0, 0, 100, 100);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
