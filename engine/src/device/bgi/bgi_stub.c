/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bgi_stub.c
 * @brief BGI Stub Implementations for Lite/Headless Build Targets.
 *
 * Provides no-op fallback implementations of all BGI functions for memory-restricted
 * build targets (like bpp and bs) where graphics rendering is disabled.
 */

#include "device/bgi.h"
#include <string.h>

static BGI_Context g_stub_bgi_ctx;

BGI_Context *BGI_get_global_context(void) {
    return &g_stub_bgi_ctx;
}

int BGI_init(BGI_Context *ctx) { (void)ctx; return 0; }
void BGI_shutdown(BGI_Context *ctx) { (void)ctx; }
int BGI_register_mode(BGI_Context *ctx, const BGI_VideoMode *mode) { (void)ctx; (void)mode; return -1; }
int BGI_set_mode(BGI_Context *ctx, int mode_handle) { (void)ctx; (void)mode_handle; return -1; }
int BGI_set_mode_by_id(BGI_Context *ctx, uint32_t mode_id) { (void)ctx; (void)mode_id; return -1; }
const uint32_t *BGI_get_framebuffer(const BGI_Context *ctx) { (void)ctx; return NULL; }
void BGI_get_dimensions(const BGI_Context *ctx, int *w, int *h) { if (w) *w = 0; if (h) *h = 0; (void)ctx; }
uint32_t BGI_resolve_color(const BGI_Context *ctx, int color) { (void)ctx; (void)color; return 0; }
void BGI_setpalette(BGI_Context *ctx, int index, uint32_t argb) { (void)ctx; (void)index; (void)argb; }
uint32_t BGI_getpalette(const BGI_Context *ctx, int index) { (void)ctx; (void)index; return 0; }
void BGI_setallpalette(BGI_Context *ctx, const uint32_t *pal, int count) { (void)ctx; (void)pal; (void)count; }
void BGI_setcolor(BGI_Context *ctx, int color) { (void)ctx; (void)color; }
void BGI_setbkcolor(BGI_Context *ctx, int color) { (void)ctx; (void)color; }
void BGI_setfillstyle(BGI_Context *ctx, int style, int color) { (void)ctx; (void)style; (void)color; }
void BGI_setlinestyle(BGI_Context *ctx, int style, int pattern, int thickness) { (void)ctx; (void)style; (void)pattern; (void)thickness; }
void BGI_putpixel(BGI_Context *ctx, int x, int y, int color) { (void)ctx; (void)x; (void)y; (void)color; }
int BGI_getpixel(const BGI_Context *ctx, int x, int y) { (void)ctx; (void)x; (void)y; return 0; }
void BGI_setviewport(BGI_Context *ctx, int x1, int y1, int x2, int y2, bool clip) { (void)ctx; (void)x1; (void)y1; (void)x2; (void)y2; (void)clip; }
void BGI_clearviewport(BGI_Context *ctx) { (void)ctx; }
void BGI_cleardevice(BGI_Context *ctx) { (void)ctx; }
void BGI_moveto(BGI_Context *ctx, int x, int y) { (void)ctx; (void)x; (void)y; }
void BGI_synthesize(BGI_Context *ctx) { (void)ctx; }
void BGI_register_heritage_modes(BGI_Context *ctx) { (void)ctx; }
void BGI_outtextxy(BGI_Context *ctx, int x, int y, const char *text) { (void)ctx; (void)x; (void)y; (void)text; }
void BGI_settextstyle(BGI_Context *ctx, int font, int direction, int size) { (void)ctx; (void)font; (void)direction; (void)size; }
int BGI_textwidth(const BGI_Context *ctx, const char *text) { (void)ctx; (void)text; return 0; }
int BGI_textheight(const BGI_Context *ctx, const char *text) { (void)ctx; (void)text; return 0; }
size_t BGI_imagesize(int x1, int y1, int x2, int y2) { (void)x1; (void)y1; (void)x2; (void)y2; return 0; }
void BGI_getimage(const BGI_Context *ctx, int x1, int y1, int x2, int y2, void *buffer) { (void)ctx; (void)x1; (void)y1; (void)x2; (void)y2; (void)buffer; }
void BGI_putimage(BGI_Context *ctx, int x, int y, const void *buffer, int op) { (void)ctx; (void)x; (void)y; (void)buffer; (void)op; }
void BGI_bar(BGI_Context *ctx, int left, int top, int right, int bottom) { (void)ctx; (void)left; (void)top; (void)right; (void)bottom; }
void BGI_rectangle(BGI_Context *ctx, int left, int top, int right, int bottom) { (void)ctx; (void)left; (void)top; (void)right; (void)bottom; }
void BGI_ellipse(BGI_Context *ctx, int x, int y, int stangle, int endangle, int xradius, int yradius) { (void)ctx; (void)x; (void)y; (void)stangle; (void)endangle; (void)xradius; (void)yradius; }

/* Fallback stubs for lite/headless targets when libadvanced / libstandard are not linked */
void vdev_gfx_force_flush(void) {}
void vdev_gfx_poll_events(void) {}
void vdev_gfx_beep(void) {}
void vdev_sound_free_all(void) {}
void vdev_image_free_all(void) {}
void vdev_play_sound_freq(double freq, double duration) { (void)freq; (void)duration; }
int  vdev_music_note_count(void) { return 0; }
int  vdev_music_queue_length(void) { return 0; }
int  stmt_edit_handler(void *vm, void *pctx) { (void)vm; (void)pctx; return 0; }
