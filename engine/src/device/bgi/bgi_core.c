/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bgi_core.c
 * @brief BGI Core Engine — Mode management, framebuffer, palette, and basic drawing.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the BGI context lifecycle (init, shutdown),
 *   video mode registration and activation, framebuffer allocation, palette
 *   management, putpixel/getpixel, viewport/clipping, and cleardevice.
 * - Why it exists: Provides a hardware-independent graphics core that any
 *   backend (SDL2, TUI, VNC, raw file) can consume without modification.
 * - Why it works this way: All VRAM is heap-allocated via calloc(). The
 *   master framebuffer is always 32-bit ARGB8888. Indexed modes store a
 *   separate 8bpp buffer that is synthesized to ARGB at flush time.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Default palette values, max mode count, synthesis
 *   algorithms for indexed-to-ARGB conversion.
 * - What cannot be changed: BGI_Context struct layout (must match bgi.h).
 * - What to expect: BGI_init() must be called before any other BGI_* calls.
 * - What to do if something breaks: Verify calloc succeeded, check mode
 *   handle bounds, ensure palette_size <= 256.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: calloc/free available. sizeof(uint32_t) == 4.
 * - Portability concerns: Pure ISO C17. No OS dependencies.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Register new modes via
 *   BGI_register_mode(). Extend synthesis for planar/beam-racing layouts.
 * - How to write external extensions: Fill BGI_VideoMode and register.
 */

#include "device/bgi.h"
#include <string.h>
#include <stdlib.h>

/* ======================================================================
 * Standard CGA/EGA/VGA 16-color palette (ARGB8888)
 * ====================================================================== */
static const uint32_t bgi_default_palette_16[16] = {
    0xFF000000u, /* 0  Black        */
    0xFF0000AAu, /* 1  Blue         */
    0xFF00AA00u, /* 2  Green        */
    0xFF00AAAAu, /* 3  Cyan         */
    0xFFAA0000u, /* 4  Red          */
    0xFFAA00AAu, /* 5  Magenta      */
    0xFFAA5500u, /* 6  Brown        */
    0xFFAAAAAAu, /* 7  Light Gray   */
    0xFF555555u, /* 8  Dark Gray    */
    0xFF5555FFu, /* 9  Light Blue   */
    0xFF55FF55u, /* 10 Light Green  */
    0xFF55FFFFu, /* 11 Light Cyan   */
    0xFFFF5555u, /* 12 Light Red    */
    0xFFFF55FFu, /* 13 Light Magenta*/
    0xFFFFFF55u, /* 14 Yellow       */
    0xFFFFFFFFu  /* 15 White        */
};

/* ======================================================================
 * Global singleton context
 * ====================================================================== */
static BGI_Context g_bgi_ctx;

BGI_Context *BGI_get_global_context(void) {
    return &g_bgi_ctx;
}

/* ======================================================================
 * Lifecycle
 * ====================================================================== */

int BGI_init(BGI_Context *ctx) {
    if (!ctx) return -1;

    memset(ctx, 0, sizeof(BGI_Context));

    /* Load default 16-color palette */
    memcpy(ctx->palette, bgi_default_palette_16, sizeof(bgi_default_palette_16));
    ctx->palette_size = 16;

    /* Default drawing state */
    ctx->fg_color = BGI_WHITE;
    ctx->bg_color = BGI_BLACK;
    ctx->fill_color = BGI_WHITE;
    ctx->fill_style = BGI_SOLID_FILL;
    ctx->line_style = BGI_SOLID_LINE;
    ctx->line_thickness = 1;

    /* Default text state */
    ctx->text_font = 0;      /* Built-in 8x8 bitmap */
    ctx->text_direction = 0;  /* Horizontal */
    ctx->text_size = 1;       /* 1x magnification */

    /* No active mode yet */
    ctx->active_mode = -1;
    ctx->active_layout = BGI_LAYOUT_LINEAR_ARGB8888;

    /* Register all heritage modes */
    BGI_register_heritage_modes(ctx);

    ctx->initialized = true;
    return 0;
}

void BGI_shutdown(BGI_Context *ctx) {
    if (!ctx) return;

    if (ctx->framebuffer) {
        free(ctx->framebuffer);
        ctx->framebuffer = NULL;
    }
    if (ctx->indexed_vram) {
        free(ctx->indexed_vram);
        ctx->indexed_vram = NULL;
    }

    /* Free any mode-owned palettes */
    for (int i = 0; i < ctx->mode_count; ++i) {
        if (ctx->modes[i].palette) {
            free(ctx->modes[i].palette);
            ctx->modes[i].palette = NULL;
        }
    }

    ctx->initialized = false;
    ctx->active_mode = -1;
    ctx->fb_width = 0;
    ctx->fb_height = 0;
}

/* ======================================================================
 * Mode Registration & Activation
 * ====================================================================== */

int BGI_register_mode(BGI_Context *ctx, const BGI_VideoMode *mode) {
    if (!ctx || !mode) return -1;
    if (ctx->mode_count >= BGI_MAX_MODES) return -1;
    if (mode->width == 0 || mode->height == 0) return -1;

    int handle = ctx->mode_count;
    memcpy(&ctx->modes[handle], mode, sizeof(BGI_VideoMode));

    /* If mode provides a palette, make our own copy */
    if (mode->palette && mode->palette_size > 0) {
        uint32_t *pal_copy = (uint32_t *)calloc(mode->palette_size,
                                                 sizeof(uint32_t));
        if (!pal_copy) return -1;
        memcpy(pal_copy, mode->palette, mode->palette_size * sizeof(uint32_t));
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

    BGI_VideoMode *mode = &ctx->modes[mode_handle];

    /* Free existing buffers */
    if (ctx->framebuffer) {
        free(ctx->framebuffer);
        ctx->framebuffer = NULL;
    }
    if (ctx->indexed_vram) {
        free(ctx->indexed_vram);
        ctx->indexed_vram = NULL;
    }

    /* Allocate 32-bit master framebuffer */
    ctx->framebuffer = (uint32_t *)calloc((size_t)mode->width * mode->height,
                                          sizeof(uint32_t));
    if (!ctx->framebuffer) return -1;

    ctx->fb_width = (int)mode->width;
    ctx->fb_height = (int)mode->height;

    /* Allocate indexed VRAM if needed */
    if (mode->mem_layout == BGI_LAYOUT_INDEXED_8BPP ||
        mode->mem_layout == BGI_LAYOUT_CELL_ATTRIBUTES) {
        ctx->indexed_vram = (uint8_t *)calloc((size_t)mode->width * mode->height,
                                               sizeof(uint8_t));
        if (!ctx->indexed_vram) {
            free(ctx->framebuffer);
            ctx->framebuffer = NULL;
            return -1;
        }
    }

    /* Load mode palette or default */
    if (mode->palette && mode->palette_size > 0) {
        int count = (int)mode->palette_size;
        if (count > 256) count = 256;
        memcpy(ctx->palette, mode->palette, (size_t)count * sizeof(uint32_t));
        ctx->palette_size = count;
    } else {
        memcpy(ctx->palette, bgi_default_palette_16,
               sizeof(bgi_default_palette_16));
        ctx->palette_size = 16;
    }

    /* Reset viewport to full screen */
    ctx->vp_left = 0;
    ctx->vp_top = 0;
    ctx->vp_right = ctx->fb_width - 1;
    ctx->vp_bottom = ctx->fb_height - 1;
    ctx->vp_clip = true;

    /* Reset cursor */
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

/* ======================================================================
 * Color Resolution
 * ====================================================================== */

uint32_t BGI_resolve_color(const BGI_Context *ctx, int color) {
    if (!ctx) return 0xFF000000u;

    /* Check for direct RGB encoding */
    if (BGI_IS_RGB_COLOR((uint32_t)color)) {
        /* Extract RGB from BGI_COLOR_RGB macro format */
        uint8_t r = (uint8_t)((color >> 16) & 0xFF);
        uint8_t g = (uint8_t)((color >>  8) & 0xFF);
        uint8_t b = (uint8_t)( color        & 0xFF);
        return 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    }

    /* Palette lookup */
    if (color >= 0 && color < ctx->palette_size) {
        return ctx->palette[color];
    }

    /* Fallback: white */
    return 0xFFFFFFFFu;
}

/* ======================================================================
 * Palette Management
 * ====================================================================== */

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
    memcpy(ctx->palette, pal, (size_t)count * sizeof(uint32_t));
    ctx->palette_size = count;
}

/* ======================================================================
 * Drawing State
 * ====================================================================== */

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
    (void)pattern; /* Reserved for future userbit pattern */
    ctx->line_style = style;
    ctx->line_thickness = thickness;
    if (ctx->line_thickness < 1) ctx->line_thickness = 1;
}

/* ======================================================================
 * Pixel Operations
 * ====================================================================== */

void BGI_putpixel(BGI_Context *ctx, int x, int y, int color) {
    if (!ctx || !ctx->framebuffer) return;

    /* Apply viewport offset */
    int px = x + ctx->vp_left;
    int py = y + ctx->vp_top;

    /* Clip */
    if (ctx->vp_clip) {
        if (px < ctx->vp_left || px > ctx->vp_right) return;
        if (py < ctx->vp_top || py > ctx->vp_bottom) return;
    }
    if (px < 0 || px >= ctx->fb_width) return;
    if (py < 0 || py >= ctx->fb_height) return;

    uint32_t argb = BGI_resolve_color(ctx, color);
    ctx->framebuffer[py * ctx->fb_width + px] = argb;

    /* Also write to indexed VRAM if active */
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

    /* If indexed VRAM exists, return index; else scan palette */
    if (ctx->indexed_vram) {
        return (int)ctx->indexed_vram[py * ctx->fb_width + px];
    }

    /* Direct ARGB: return as-is with high bit set */
    return (int)ctx->framebuffer[py * ctx->fb_width + px];
}

/* ======================================================================
 * Viewport & Clearing
 * ====================================================================== */

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
        memset(ctx->indexed_vram, (uint8_t)ctx->bg_color,
               (size_t)ctx->fb_width * (size_t)ctx->fb_height);
    }
    ctx->cp_x = 0;
    ctx->cp_y = 0;
}

/* ======================================================================
 * Cursor Movement
 * ====================================================================== */

void BGI_moveto(BGI_Context *ctx, int x, int y) {
    if (!ctx) return;
    ctx->cp_x = x;
    ctx->cp_y = y;
}

/* ======================================================================
 * Synthesis — Convert indexed VRAM to ARGB master framebuffer
 * ====================================================================== */

void BGI_synthesize(BGI_Context *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    /* Only needed for indexed modes */
    if (ctx->active_layout == BGI_LAYOUT_INDEXED_8BPP && ctx->indexed_vram) {
        size_t total = (size_t)ctx->fb_width * (size_t)ctx->fb_height;
        for (size_t i = 0; i < total; ++i) {
            uint8_t idx = ctx->indexed_vram[i];
            ctx->framebuffer[i] = (idx < (uint8_t)ctx->palette_size)
                ? ctx->palette[idx]
                : 0xFF000000u;
        }
    }
    /* CELL_ATTRIBUTES and PLANAR modes would add synthesis here */
    /* BEAM_RACING modes would drive synthesis from scanline callbacks */
}
