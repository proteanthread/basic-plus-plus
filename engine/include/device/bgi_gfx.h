// FILENAME: bgi_gfx.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (compat.c, context.c, screen.c)
// NEEDED BY: libkernel (bgi_bridge.c, bgi_gfx.c)
// NEEDS: libengine (bgi.h, bgi.c, bios.h, bios.c)
// NEEDS: libkernel (bgi_palette.h, bgi_palette.c)
// Implements virtual device and graphics rendering logic for bgi_gfx.
//
// ---- Includes ----

#ifndef DEVICE_BGI_GFX_H
#define DEVICE_BGI_GFX_H

#include "device/bgi.h"
#include "bios/bios.h"
#include "device/bgi_palette.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BGI_GFX_MAX_SCANLINES 2160

// @brief Graphics VRAM Observer Context.
typedef struct BgiGfxContext {
    BiosContext *bios;                     // < Target BIOS context
    BGI_Context *bgi;                      // < Target BGI graphics context
    bool         dirty_lines[BGI_GFX_MAX_SCANLINES]; // < Dirty flags per scanline
    bool         any_dirty;                // < True if at least one line is dirty
    int          active_mode;              // < Active SCREEN mode number
    bool         active;                   // < True if graphics observer is enabled
} BgiGfxContext;

// @brief Initialize BGI graphics-mode VRAM observer.
void bgi_gfx_init(BiosContext *bios, BGI_Context *bgi);

// @brief Shutdown BGI graphics-mode VRAM observer.
void bgi_gfx_shutdown(BiosContext *bios);

// @brief Enable or set the active graphics SCREEN mode for VRAM decoding.
void bgi_gfx_set_screen_mode(int mode);

// @brief Activate a custom resolution graphics mode dynamically.
void bgi_gfx_set_screen_mode_custom(int width, int height, int bpp, int text_cols, int text_rows, float fps);

// @brief Mark a single scanline dirty for rendering.
void bgi_gfx_mark_dirty_scanline(int scanline);

// @brief Sweep and decode all dirty scanlines to the BGI master canvas.
void bgi_gfx_sweep_dirty_scanlines(void);

// @brief Force a full refresh of all scanlines.
void bgi_gfx_force_refresh(void);

// @brief Get global BGI graphics observer context instance.
BgiGfxContext* bgi_gfx_get_global(void);

#endif // DEVICE_BGI_GFX_H
