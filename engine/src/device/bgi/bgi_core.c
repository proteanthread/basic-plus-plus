// FILENAME: bgi_core.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (hal.h, memops.h, memops.c, strops.h, strops.c)
// NEEDS: libengine (bgi.h, bgi.c)
// Implements virtual device and graphics rendering logic for bgi_core.
//
// ---- Includes ----

#include "device/bgi.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

// ======================================================================
// Standard CGA/EGA/VGA 16-color palette (ARGB8888)
// ======================================================================
static const uint32_t bgi_default_palette_16[16] = {
    0xFF000000u, // 0  Black
    0xFF0000AAu, // 1  Blue
    0xFF00AA00u, // 2  Green
    0xFF00AAAAu, // 3  Cyan
    0xFFAA0000u, // 4  Red
    0xFFAA00AAu, // 5  Magenta
    0xFFAA5500u, // 6  Brown
    0xFFAAAAAAu, // 7  Light Gray
    0xFF555555u, // 8  Dark Gray
    0xFF5555FFu, // 9  Light Blue
    0xFF55FF55u, // 10 Light Green
    0xFF55FFFFu, // 11 Light Cyan
    0xFFFF5555u, // 12 Light Red
    0xFFFF55FFu, // 13 Light Magenta
    0xFFFFFF55u, // 14 Yellow
    0xFFFFFFFFu  // 15 White
};

// ======================================================================
// Global singleton context
// ======================================================================
static BGI_Context g_bgi_ctx;

BGI_Context *BGI_get_global_context(void) {
    return &g_bgi_ctx;
}

// ======================================================================
// Lifecycle
// ======================================================================

int BGI_init(BGI_Context *ctx) {
    if (!ctx) return -1;

    runtime_memset(ctx, 0, sizeof(BGI_Context));

    // Load default 16-color palette
    runtime_memcpy(ctx->palette, bgi_default_palette_16, sizeof(bgi_default_palette_16));
    ctx->palette_size = 16;

    // Default drawing state
    ctx->fg_color = BGI_WHITE;
    ctx->bg_color = BGI_BLACK;
    ctx->fill_color = BGI_WHITE;
    ctx->fill_style = BGI_SOLID_FILL;
    ctx->line_style = BGI_SOLID_LINE;
    ctx->line_thickness = 1;

    // Default text state
    ctx->text_font = 0;      // Built-in 8x8 bitmap
    ctx->text_direction = 0;  // Horizontal
    ctx->text_size = 1;       // 1x magnification

    // No active mode yet
    ctx->active_mode = -1;
    ctx->active_layout = BGI_LAYOUT_LINEAR_ARGB8888;

    // Register all heritage modes
    BGI_register_heritage_modes(ctx);

    ctx->initialized = true;
    return 0;
}

void BGI_shutdown(BGI_Context *ctx) {
    if (!ctx) return;

    HalContext *hal = hal_get();

    if (ctx->framebuffer) {
        if (hal && hal->mem.free) hal->mem.free(ctx->framebuffer);
        ctx->framebuffer = NULL;
    }
    if (ctx->indexed_vram) {
        if (hal && hal->mem.free) hal->mem.free(ctx->indexed_vram);
        ctx->indexed_vram = NULL;
    }

    // Free any mode-owned palettes
    for (int i = 0; i < ctx->mode_count; ++i) {
        if (ctx->modes[i].palette) {
            if (hal && hal->mem.free) hal->mem.free(ctx->modes[i].palette);
            ctx->modes[i].palette = NULL;
        }
    }

    ctx->initialized = false;
    ctx->active_mode = -1;
    ctx->fb_width = 0;
    ctx->fb_height = 0;
}

// ======================================================================
// Mode Registration & Activation
// ======================================================================

int BGI_register_mode(BGI_Context *ctx, const BGI_VideoMode *mode) {
    if (!ctx || !mode) return -1;
    if (ctx->mode_count >= BGI_MAX_MODES) return -1;
    if (mode->width == 0 || mode->height == 0) return -1;

    HalContext *hal = hal_get();
    int handle = ctx->mode_count;
    runtime_memcpy(&ctx->modes[handle], mode, sizeof(BGI_VideoMode));

    // If mode provides a palette, make our own copy
    if (mode->palette && mode->palette_size > 0) {
        size_t pal_bytes = (size_t)mode->palette_size * sizeof(uint32_t);
        uint32_t *pal_copy = (uint32_t *)(hal && hal->mem.alloc ? hal->mem.alloc(pal_bytes) : NULL);
        if (!pal_copy) return -1;
        runtime_memcpy(pal_copy, mode->palette, pal_bytes);
        ctx->modes[handle].palette = pal_copy;
    } else {
        ctx->modes[handle].palette = NULL;
    }

    ctx->mode_count++;
    return handle;
}

int BGI_set_mode(BGI_Context *ctx, int mode_handle) {
    if (!ctx) return -1;
    if (mode_handle < 0 || mode_handle >= ctx->mode_count) return -1;

    HalContext *hal = hal_get();
    BGI_VideoMode *mode = &ctx->modes[mode_handle];

    // Free existing buffers
    if (ctx->framebuffer) {
        if (hal && hal->mem.free) hal->mem.free(ctx->framebuffer);
        ctx->framebuffer = NULL;
    }
    if (ctx->indexed_vram) {
        if (hal && hal->mem.free) hal->mem.free(ctx->indexed_vram);
        ctx->indexed_vram = NULL;
    }

    // Allocate 32-bit master framebuffer
    size_t fb_bytes = (size_t)mode->width * (size_t)mode->height * sizeof(uint32_t);
    ctx->framebuffer = (uint32_t *)(hal && hal->mem.alloc ? hal->mem.alloc(fb_bytes) : NULL);
    if (!ctx->framebuffer) return -1;
    runtime_memset(ctx->framebuffer, 0, fb_bytes);

    ctx->fb_width = (int)mode->width;
    ctx->fb_height = (int)mode->height;

    // Allocate indexed VRAM if needed
    if (mode->mem_layout == BGI_LAYOUT_INDEXED_8BPP ||
        mode->mem_layout == BGI_LAYOUT_CELL_ATTRIBUTES) {
        size_t vram_bytes = (size_t)mode->width * (size_t)mode->height * sizeof(uint8_t);
        ctx->indexed_vram = (uint8_t *)(hal && hal->mem.alloc ? hal->mem.alloc(vram_bytes) : NULL);
        if (!ctx->indexed_vram) {
            if (hal && hal->mem.free) hal->mem.free(ctx->framebuffer);
            ctx->framebuffer = NULL;
            return -1;
        }
        runtime_memset(ctx->indexed_vram, 0, vram_bytes);
    }

    // Load mode palette or default
    if (mode->palette && mode->palette_size > 0) {
        int count = (int)mode->palette_size;
        if (count > 256) count = 256;
        runtime_memcpy(ctx->palette, mode->palette, (size_t)count * sizeof(uint32_t));
        ctx->palette_size = count;
    } else {
        runtime_memcpy(ctx->palette, bgi_default_palette_16,
                       sizeof(bgi_default_palette_16));
        ctx->palette_size = 16;
    }


    // Reset viewport to full screen
    ctx->vp_left = 0;
    ctx->vp_top = 0;
    ctx->vp_right = ctx->fb_width - 1;
    ctx->vp_bottom = ctx->fb_height - 1;
    ctx->vp_clip = true;

    // Reset cursor
    ctx->cp_x = 0;
    ctx->cp_y = 0;

    ctx->active_mode = mode_handle;
    ctx->active_layout = mode->mem_layout;

    return 0;
}

int BGI_set_mode_by_id(BGI_Context *ctx, uint32_t mode_id) {
    if (!ctx) return -1;
    for (int i = 0; i < ctx->mode_count; ++i) {
        if (ctx->modes[i].mode_id == mode_id) {
            return BGI_set_mode(ctx, i);
        }
    }
    return -1;
}

const uint32_t *BGI_get_framebuffer(const BGI_Context *ctx) {
    if (!ctx) return NULL;
    return ctx->framebuffer;
}

void BGI_get_dimensions(const BGI_Context *ctx, int *w, int *h) {
    if (!ctx) {
        if (w) *w = 0;
        if (h) *h = 0;
        return;
    }
    if (w) *w = ctx->fb_width;
    if (h) *h = ctx->fb_height;
}

// ======================================================================
// Color Resolution
// ======================================================================

uint32_t BGI_resolve_color(const BGI_Context *ctx, int color) {
    if (!ctx) return 0xFF000000u;

    // Check for direct RGB encoding
    if (BGI_IS_RGB_COLOR((uint32_t)color)) {
        // Extract RGB from BGI_COLOR_RGB macro format
        uint8_t r = (uint8_t)((color >> 16) & 0xFF);
        uint8_t g = (uint8_t)((color >>  8) & 0xFF);
        uint8_t b = (uint8_t)( color        & 0xFF);
        return 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    }

    // Palette lookup
    if (color >= 0 && color < ctx->palette_size) {
        return ctx->palette[color];
    }

    // Fallback: white
    return 0xFFFFFFFFu;
}

// ======================================================================
// Palette Management
// ======================================================================

void BGI_setpalette(BGI_Context *ctx, int index, uint32_t argb) {
    if (!ctx || index < 0 || index >= 256) return;
    ctx->palette[index] = argb;
    if (index >= ctx->palette_size) {
        ctx->palette_size = index + 1;
    }
}

uint32_t BGI_getpalette(const BGI_Context *ctx, int index) {
    if (!ctx || index < 0 || index >= 256) return 0;
    return ctx->palette[index];
}

void BGI_setallpalette(BGI_Context *ctx, const uint32_t *pal, int count) {
    if (!ctx || !pal || count <= 0) return;
    if (count > 256) count = 256;
    runtime_memcpy(ctx->palette, pal, (size_t)count * sizeof(uint32_t));
    ctx->palette_size = count;
}


// ======================================================================
// Drawing State
// ======================================================================

void BGI_setcolor(BGI_Context *ctx, int color) {
    if (!ctx) return;
    ctx->fg_color = color;
}

void BGI_setbkcolor(BGI_Context *ctx, int color) {
    if (!ctx) return;
    ctx->bg_color = color;
}

void BGI_setfillstyle(BGI_Context *ctx, int style, int color) {
    if (!ctx) return;
    ctx->fill_style = style;
    ctx->fill_color = color;
}

void BGI_setlinestyle(BGI_Context *ctx, int style, int pattern, int thickness) {
    if (!ctx) return;
    (void)pattern; // Reserved for future userbit pattern
    ctx->line_style = style;
    ctx->line_thickness = thickness;
    if (ctx->line_thickness < 1) ctx->line_thickness = 1;
}

// ======================================================================
// Pixel Operations
// ======================================================================

void BGI_putpixel(BGI_Context *ctx, int x, int y, int color) {
    if (!ctx || !ctx->framebuffer) return;

    // Apply viewport offset
    int px = x + ctx->vp_left;
    int py = y + ctx->vp_top;

    // Clip
    if (ctx->vp_clip) {
        if (px < ctx->vp_left || px > ctx->vp_right) return;
        if (py < ctx->vp_top || py > ctx->vp_bottom) return;
    }
    if (px < 0 || px >= ctx->fb_width) return;
    if (py < 0 || py >= ctx->fb_height) return;

    uint32_t argb = BGI_resolve_color(ctx, color);
    ctx->framebuffer[py * ctx->fb_width + px] = argb;

    // Also write to indexed VRAM if active
    if (ctx->indexed_vram && color >= 0 && color < 256) {
        ctx->indexed_vram[py * ctx->fb_width + px] = (uint8_t)color;
    }
}

int BGI_getpixel(const BGI_Context *ctx, int x, int y) {
    if (!ctx || !ctx->framebuffer) return 0;

    int px = x + ctx->vp_left;
    int py = y + ctx->vp_top;

    if (px < 0 || px >= ctx->fb_width) return 0;
    if (py < 0 || py >= ctx->fb_height) return 0;

    // If indexed VRAM exists, return index; else scan palette
    if (ctx->indexed_vram) {
        return (int)ctx->indexed_vram[py * ctx->fb_width + px];
    }

    // Direct ARGB: return as-is with high bit set
    return (int)ctx->framebuffer[py * ctx->fb_width + px];
}

// ======================================================================
// Viewport & Clearing
// ======================================================================

void BGI_setviewport(BGI_Context *ctx, int x1, int y1, int x2, int y2,
                     bool clip) {
    if (!ctx) return;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= ctx->fb_width) x2 = ctx->fb_width - 1;
    if (y2 >= ctx->fb_height) y2 = ctx->fb_height - 1;
    if (x1 > x2 || y1 > y2) return;

    ctx->vp_left = x1;
    ctx->vp_top = y1;
    ctx->vp_right = x2;
    ctx->vp_bottom = y2;
    ctx->vp_clip = clip;
    ctx->cp_x = 0;
    ctx->cp_y = 0;
}

void BGI_clearviewport(BGI_Context *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    uint32_t bg = BGI_resolve_color(ctx, ctx->bg_color);
    for (int y = ctx->vp_top; y <= ctx->vp_bottom; ++y) {
        for (int x = ctx->vp_left; x <= ctx->vp_right; ++x) {
            if (x >= 0 && x < ctx->fb_width && y >= 0 && y < ctx->fb_height) {
                ctx->framebuffer[y * ctx->fb_width + x] = bg;
                if (ctx->indexed_vram) {
                    ctx->indexed_vram[y * ctx->fb_width + x] =
                        (uint8_t)ctx->bg_color;
                }
            }
        }
    }
}

void BGI_cleardevice(BGI_Context *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    uint32_t bg = BGI_resolve_color(ctx, ctx->bg_color);
    size_t total = (size_t)ctx->fb_width * (size_t)ctx->fb_height;
    for (size_t i = 0; i < total; ++i) {
        ctx->framebuffer[i] = bg;
    }
    if (ctx->indexed_vram) {
        runtime_memset(ctx->indexed_vram, (uint8_t)ctx->bg_color,
                       (size_t)ctx->fb_width * (size_t)ctx->fb_height);
    }

    ctx->cp_x = 0;
    ctx->cp_y = 0;
}

// ======================================================================
// Cursor Movement
// ======================================================================

void BGI_moveto(BGI_Context *ctx, int x, int y) {
    if (!ctx) return;
    ctx->cp_x = x;
    ctx->cp_y = y;
}

// ======================================================================
// Synthesis — Convert indexed VRAM to ARGB master framebuffer
// ======================================================================

void BGI_synthesize(BGI_Context *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    // Only needed for indexed modes
    if (ctx->active_layout == BGI_LAYOUT_INDEXED_8BPP && ctx->indexed_vram) {
        size_t total = (size_t)ctx->fb_width * (size_t)ctx->fb_height;
        for (size_t i = 0; i < total; ++i) {
            uint8_t idx = ctx->indexed_vram[i];
            ctx->framebuffer[i] = (idx < (uint8_t)ctx->palette_size)
                ? ctx->palette[idx]
                : 0xFF000000u;
        }
    }
    // CELL_ATTRIBUTES and PLANAR modes would add synthesis here
    // BEAM_RACING modes would drive synthesis from scanline callbacks
}

// ======================================================================
// Display Sync Hook
// ======================================================================
static void (*s_bgi_sync_hook)(void) = NULL;

void bgi_set_sync_hook(void (*hook)(void)) {
    s_bgi_sync_hook = hook;
}

void bgi_sync(void) {
    if (s_bgi_sync_hook) {
        s_bgi_sync_hook();
    }
}

// ======================================================================
// Window Presentation and HAL Routing
// ======================================================================

bool BGI_init_window(BGI_Context *ctx, const char *title) {
    if (!ctx || ctx->fb_width <= 0 || ctx->fb_height <= 0) return false;
    HalContext *hal = hal_get();
    if (hal && hal->video.init) {
        return hal->video.init(ctx->fb_width, ctx->fb_height, 32, title ? title : "BASIC++ Graphics Display");
    }
    return false;
}

void BGI_shutdown_window(BGI_Context *ctx) {
    (void)ctx;
    HalContext *hal = hal_get();
    if (hal && hal->video.shutdown) {
        hal->video.shutdown();
    }
}

void BGI_present(BGI_Context *ctx) {
    if (!ctx || !ctx->framebuffer || ctx->fb_width <= 0 || ctx->fb_height <= 0) return;
    HalContext *hal = hal_get();
    if (hal && hal->video.present_framebuffer) {
        hal->video.present_framebuffer(ctx->framebuffer, ctx->fb_width, ctx->fb_height, ctx->fb_width * (int)sizeof(uint32_t));
    }
    if (hal && hal->video.poll_events) {
        hal->video.poll_events();
    }
    bgi_sync();
}

void BGI_poll_events(BGI_Context *ctx) {
    (void)ctx;
    HalContext *hal = hal_get();
    if (hal && hal->video.poll_events) {
        hal->video.poll_events();
    }
}

bool BGI_is_window_open(const BGI_Context *ctx) {
    (void)ctx;
    HalContext *hal = hal_get();
    if (hal && hal->video.is_window_open) {
        return hal->video.is_window_open();
    }
    return false;
}

// ======================================================================
// Split-Screen Text Window Rendering
// ======================================================================

void BGI_draw_split_text(BGI_Context *ctx, int col, int row, const char *text, int scale, int color) {
    if (!ctx || !ctx->framebuffer || !text) return;
    if (scale <= 0) scale = 1;
    if (scale > 4) scale = 4;

    const uint8_t *font = BGI_get_font_8x8();
    if (!font) return;

    int glyph_w = 8 * scale;
    int glyph_h = 8 * scale;

    int text_area_top = ctx->fb_height - (ctx->text_window_rows > 0 ? (ctx->text_window_rows * glyph_h) : (4 * glyph_h));
    if (text_area_top < 0) text_area_top = 0;

    int base_x = col * glyph_w;
    int base_y = text_area_top + row * glyph_h;

    uint32_t fg_argb = BGI_resolve_color(ctx, color);

    for (size_t i = 0; text[i] != '\0'; ++i) {
        uint8_t ch = (uint8_t)text[i];
        int gx = base_x + (int)i * glyph_w;
        if (gx + glyph_w > ctx->fb_width) break;

        const uint8_t *glyph_data = &font[ch * 8];
        for (int gy = 0; gy < 8; ++gy) {
            uint8_t row_bits = glyph_data[gy];
            for (int bit = 0; bit < 8; ++bit) {
                if ((row_bits >> (7 - bit)) & 1) {
                    for (int sy = 0; sy < scale; ++sy) {
                        for (int sx = 0; sx < scale; ++sx) {
                            int px = gx + bit * scale + sx;
                            int py = base_y + gy * scale + sy;
                            if (px >= 0 && px < ctx->fb_width && py >= 0 && py < ctx->fb_height) {
                                ctx->framebuffer[py * ctx->fb_width + px] = fg_argb;
                            }
                        }
                    }
                }
            }
        }
    }
}


