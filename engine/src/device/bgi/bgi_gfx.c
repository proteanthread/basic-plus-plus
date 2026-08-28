// FILENAME: bgi_gfx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (compat.c, context.c, screen.c)
// NEEDED BY: libkernel (bgi_bridge.c)
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libkernel (bgi_gfx.h, bgi_palette.h, bgi_palette.c)
// Implements virtual device and graphics rendering logic for bgi_gfx.
//
// ---- Includes ----

#include "device/bgi_gfx.h"
#include "device/bgi_palette.h"
#include "runtime/string/memops.h"

static BgiGfxContext g_gfx_context;


// ======================================================================
// Scanline Decoders for Heritage Video Memory Layouts
// ======================================================================

// @brief Decode CGA 320x200 4-color interlaced scanline (SCREEN 1).
// Bank 0 (even scanlines): 0xB8000 - 0xB9F3F (8000 bytes, 80 bytes/line * 100 lines)
// Bank 1 (odd scanlines) : 0xBA000 - 0xBBF3F (8000 bytes, 80 bytes/line * 100 lines)
static void decode_scanline_cga_1(BgiGfxContext *ctx, int y) {
    if (!ctx || !ctx->bios || !ctx->bgi || !ctx->bgi->framebuffer) return;
    if (y < 0 || y >= 200) return;

    uint32_t bank_offset = (y % 2 == 0) ? 0xB8000U : 0xBA000U;
    uint32_t line_offset = bank_offset + (uint32_t)((y / 2) * 80);
    BgiPalette *pal = bgi_palette_get_global();

    uint32_t *dst_row = ctx->bgi->framebuffer + (y * ctx->bgi->fb_width);

    for (int col_byte = 0; col_byte < 80; ++col_byte) {
        uint8_t byte_val = bios_peek(ctx->bios, line_offset + (uint32_t)col_byte);

        // 4 pixels per byte in 2bpp mode: bits 7-6, 5-4, 3-2, 1-0
        for (int p = 0; p < 4; ++p) {
            int px = col_byte * 4 + p;
            if (px >= 320) break;
            uint8_t color_idx = (byte_val >> (6 - p * 2)) & 0x03;
            dst_row[px] = bgi_palette_get_argb(pal, color_idx);
        }
    }
}

// @brief Decode CGA 640x200 2-color interlaced scanline (SCREEN 2).
static void decode_scanline_cga_2(BgiGfxContext *ctx, int y) {
    if (!ctx || !ctx->bios || !ctx->bgi || !ctx->bgi->framebuffer) return;
    if (y < 0 || y >= 200) return;

    uint32_t bank_offset = (y % 2 == 0) ? 0xB8000U : 0xBA000U;
    uint32_t line_offset = bank_offset + (uint32_t)((y / 2) * 80);
    BgiPalette *pal = bgi_palette_get_global();

    uint32_t *dst_row = ctx->bgi->framebuffer + (y * ctx->bgi->fb_width);

    for (int col_byte = 0; col_byte < 80; ++col_byte) {
        uint8_t byte_val = bios_peek(ctx->bios, line_offset + (uint32_t)col_byte);

        // 8 pixels per byte in 1bpp mode: bits 7 down to 0
        for (int p = 0; p < 8; ++p) {
            int px = col_byte * 8 + p;
            if (px >= 640) break;
            uint8_t color_idx = (byte_val >> (7 - p)) & 0x01;
            dst_row[px] = bgi_palette_get_argb(pal, color_idx ? 15 : 0);
        }
    }
}

// @brief Decode VGA Mode 13h 320x200 256-color linear chunky scanline (SCREEN 13).
// Linear buffer at 0xA0000 (320 bytes per scanline).
static void decode_scanline_vga13(BgiGfxContext *ctx, int y) {
    if (!ctx || !ctx->bios || !ctx->bgi || !ctx->bgi->framebuffer) return;
    if (y < 0 || y >= 200) return;

    uint32_t line_offset = 0xA0000U + (uint32_t)(y * 320);
    BgiPalette *pal = bgi_palette_get_global();

    uint32_t *dst_row = ctx->bgi->framebuffer + (y * ctx->bgi->fb_width);

    for (int x = 0; x < 320; ++x) {
        uint8_t color_idx = bios_peek(ctx->bios, line_offset + (uint32_t)x);
        dst_row[x] = bgi_palette_get_argb(pal, color_idx);
    }
}

// ======================================================================
// VRAM Observer Callback
// ======================================================================

static void vram_gfx_write_cb(void *user_data, uint32_t phys_addr, uint8_t value) {
    (void)user_data;
    (void)value;
    BgiGfxContext *ctx = &g_gfx_context;
    if (!ctx->active) return;

    int y = -1;
    if (phys_addr >= BIOS_VRAM_CGA_START && phys_addr <= BIOS_VRAM_CGA_END) {
        // CGA graphics region
        uint32_t rel = phys_addr - BIOS_VRAM_CGA_START;
        if (rel < 8000U) {
            // Even scanlines
            y = (int)(rel / 80) * 2;
        } else if (rel >= 0x2000U && rel < 0x2000U + 8000U) {
            // Odd scanlines
            y = (int)((rel - 0x2000U) / 80) * 2 + 1;
        }
    } else if (phys_addr >= BIOS_VRAM_EGA_START && phys_addr <= BIOS_VRAM_EGA_END) {
        // EGA/VGA graphics region
        uint32_t rel = phys_addr - BIOS_VRAM_EGA_START;
        if (ctx->active_mode == 13) {
            y = (int)(rel / 320);
        } else {
            y = (int)(rel / 80); // 640-pixel modes: 80 bytes per line
        }
    }

    if (y >= 0 && y < BGI_GFX_MAX_SCANLINES) {
        ctx->dirty_lines[y] = true;
        ctx->any_dirty = true;
    }
}

// ======================================================================
// Public API
// ======================================================================

void bgi_gfx_init(BiosContext *bios, BGI_Context *bgi) {
    runtime_memset(&g_gfx_context, 0, sizeof(g_gfx_context));
    g_gfx_context.bios = bios;
    g_gfx_context.bgi = bgi;
    g_gfx_context.active = false;

    if (bios) {
        BiosVRAMObserver observer;
        runtime_memset(&observer, 0, sizeof(observer));
        observer.user_data = &g_gfx_context;
        observer.on_write = vram_gfx_write_cb;
        bios_set_vram_observer(bios, &observer);
    }
}


void bgi_gfx_shutdown(BiosContext *bios) {
    if (bios) {
        bios_set_vram_observer(bios, NULL);
    }
    g_gfx_context.active = false;
}

void bgi_gfx_set_screen_mode(int mode) {
    g_gfx_context.active_mode = mode;
    g_gfx_context.active = (mode > 0);
    bgi_gfx_force_refresh();
}

void bgi_gfx_set_screen_mode_custom(int width, int height, int bpp, int text_cols, int text_rows, float fps) {
    BgiGfxContext *ctx = &g_gfx_context;
    if (ctx->bgi) {
        BGI_create_custom_mode(ctx->bgi, width, height, bpp, text_cols, text_rows, fps);
    }
    ctx->active_mode = -1; // Custom mode flag
    ctx->active = true;
    bgi_gfx_force_refresh();
}

void bgi_gfx_mark_dirty_scanline(int scanline) {
    if (scanline >= 0 && scanline < BGI_GFX_MAX_SCANLINES) {
        g_gfx_context.dirty_lines[scanline] = true;
        g_gfx_context.any_dirty = true;
    }
}

void bgi_gfx_sweep_dirty_scanlines(void) {
    BgiGfxContext *ctx = &g_gfx_context;
    if (!ctx->active || !ctx->any_dirty || !ctx->bgi) return;

    int max_y = (ctx->bgi->fb_height < BGI_GFX_MAX_SCANLINES) ? ctx->bgi->fb_height : BGI_GFX_MAX_SCANLINES;

    for (int y = 0; y < max_y; ++y) {
        if (!ctx->dirty_lines[y]) continue;

        switch (ctx->active_mode) {
            case 1:
                decode_scanline_cga_1(ctx, y);
                break;
            case 2:
                decode_scanline_cga_2(ctx, y);
                break;
            case 13:
                decode_scanline_vga13(ctx, y);
                break;
            default:
                break;
        }

        ctx->dirty_lines[y] = false;
    }

    ctx->any_dirty = false;
}

void bgi_gfx_force_refresh(void) {
    BgiGfxContext *ctx = &g_gfx_context;
    if (!ctx->bgi) return;

    int max_y = (ctx->bgi->fb_height < BGI_GFX_MAX_SCANLINES) ? ctx->bgi->fb_height : BGI_GFX_MAX_SCANLINES;
    for (int y = 0; y < max_y; ++y) {
        ctx->dirty_lines[y] = true;
    }
    ctx->any_dirty = true;
    bgi_gfx_sweep_dirty_scanlines();
}

BgiGfxContext* bgi_gfx_get_global(void) {
    return &g_gfx_context;
}
