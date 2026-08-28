# C17 API Reference: BASIC++ Graphics Interface (`device/bgi.h`)

## 1. Subsystem Overview & Responsibilities

The BASIC++ Graphics Interface (`device/bgi.h`, implemented in `engine/src/device/bgi.c`) provides the software rasterizer, 42 retro hardware mode profiles, geometric rendering primitives, palettes, viewports, and split-screen text window rendering for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Freestanding C17 Rasterizer**: Pure software framebuffer rendering operating independently of host graphics APIs, with optional delay-loaded SDL2 bindings in `baspp`.
- **Tri-Mode Memory Layout**: Models direct-color (`LINEAR_ARGB8888`), chunky indexed (`INDEXED_8BPP`), planar EGA/VGA (`PLANAR_4PLANE`), and cell attribute text displays (`CELL_ATTRIBUTES`).
- **42 Retro Mode Profiles**: Maps retro video standards including CGA, EGA, VGA, MCGA, Hercules, PCjr, Tandy 1000, Apple II Hi-Res/Double Hi-Res, Commodore 64 VIC-II, Atari 800 GTIA, and Amiga OCS.
- **Bresenham Primitives & Fast Flood Fill**: Optimized integer line drawing, circles, ellipses, filled polygons, and scanline flood fills.
- **Bi-Directional BIOS Video Bridge**: Coordinates with `libbios` via `BiosVRAMObserver` callbacks to trap direct VRAM writes (`0xB8000`, `0xA0000`) and sync CRTC display registers.

## 2. Header Inclusion & Prerequisites

```c
#include "device/bgi.h"
#include <stdint.h>
#include <stdbool.h>
```

## 3. Data Structures & Types

```c
/* VRAM Physical Memory Layout */
typedef enum BGI_MemLayout {
    BGI_LAYOUT_LINEAR_ARGB8888  = 0, /* 32-bit direct-color framebuffer */
    BGI_LAYOUT_INDEXED_8BPP     = 1, /* 8bpp chunky palette framebuffer */
    BGI_LAYOUT_PLANAR_4PLANE    = 2, /* 4-plane EGA/VGA planar layout    */
    BGI_LAYOUT_CELL_ATTRIBUTES  = 3, /* Character + attribute text mode */
    BGI_LAYOUT_BEAM_RACING      = 4  /* Scanline-driven, no framebuffer */
} BGI_MemLayout;

/* Complete Video Mode Descriptor */
typedef struct BGI_VideoMode {
    char          mode_name[64];     /* 7-bit ASCII name (e.g. "VGA_320x200x256") */
    uint32_t      mode_id;           /* Unique numeric identifier */
    uint32_t      width;             /* Pixel width */
    uint32_t      height;            /* Pixel height */
    uint8_t       bits_per_pixel;    /* 1, 2, 4, 8, 16, 24, 32 */
    uint32_t      palette_size;      /* Color count (0=direct) */
    uint32_t     *palette;           /* ARGB8888 palette table */
    BGI_MemLayout mem_layout;        /* Framebuffer mapping */
    uint8_t       cell_width;        /* Character cell width */
    uint8_t       cell_height;       /* Character cell height */
    float         aspect_ratio;      /* e.g. 4.0f / 3.0f */
    float         refresh_hz;        /* Refresh rate (60.0f, 70.0f) */
} BGI_VideoMode;

/* Opaque BGI Context */
typedef struct BGIContext BGIContext;
```

## 4. Function Prototypes & Operational Contracts

### Context Lifecycle & Mode Control
```c
/**
 * @brief Initializes the BGI graphics rasterizer context.
 */
BGIContext *bgi_init(void);

/**
 * @brief Destroys the BGI context and frees framebuffers.
 */
void bgi_shutdown(BGIContext *ctx);

/**
 * @brief Sets the active video mode by ID or resolution profile.
 */
bool bgi_set_mode(BGIContext *ctx, uint32_t mode_id);
bool bgi_set_screen(BGIContext *ctx, uint32_t w, uint32_t h, uint8_t bpp, uint32_t text_cols, uint32_t text_rows);
```

### Geometric Drawing Primitives
```c
/**
 * @brief Plots a single pixel at (x, y) with color attribute.
 */
void bgi_put_pixel(BGIContext *ctx, int x, int y, uint32_t color);

/**
 * @brief Reads the color attribute of a pixel at (x, y) (POINT function).
 */
uint32_t bgi_get_pixel(BGIContext *ctx, int x, int y);

/**
 * @brief Draws a line between two coordinates using Bresenham's algorithm.
 */
void bgi_line(BGIContext *ctx, int x1, int y1, int x2, int y2, uint32_t color);

/**
 * @brief Draws a rectangle outline or solid filled box.
 */
void bgi_rectangle(BGIContext *ctx, int x1, int y1, int x2, int y2, uint32_t color, bool filled);

/**
 * @brief Draws a circle outline or filled disc.
 */
void bgi_circle(BGIContext *ctx, int cx, int cy, int radius, uint32_t color, bool filled);

/**
 * @brief Draws an ellipse or elliptical arc.
 */
void bgi_ellipse(BGIContext *ctx, int cx, int cy, int rx, int ry, uint32_t color, bool filled);

/**
 * @brief Performs boundary or surface flood fill starting at (x, y).
 */
void bgi_flood_fill(BGIContext *ctx, int x, int y, uint32_t fill_color, uint32_t boundary_color);
```

### Viewports & Palettes
```c
/**
 * @brief Sets the active graphics clipping viewport (VIEW statement).
 */
void bgi_set_viewport(BGIContext *ctx, int x1, int y1, int x2, int y2, bool clip);

/**
 * @brief Sets an individual palette color register (PALETTE statement).
 */
void bgi_set_palette_entry(BGIContext *ctx, uint32_t index, uint32_t argb);
```

## 5. Architectural Invariants

- **Freestanding Safety**: Core rasterizer requires zero libc dependencies beyond standard integer headers.
- **Clipping Protection**: All drawing primitives enforce clipping against the active viewport rectangle to prevent buffer overruns.

## 6. Code Example: Initializing Mode 13h and Drawing Primitives

```c
#include "device/bgi.h"

void draw_retro_scene(void) {
    BGIContext *bgi = bgi_init();
    
    /* Set standard VGA 320x200 256-color mode */
    bgi_set_mode(bgi, 0x13);

    /* Draw border and center circle */
    bgi_rectangle(bgi, 0, 0, 319, 199, 14, false);
    bgi_circle(bgi, 160, 100, 50, 9, true);

    bgi_shutdown(bgi);
}
```
