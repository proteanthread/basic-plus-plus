// FILENAME: dev_framebuffer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (dev_framebuffer.h, vdev.h)
// Implementation for 4-bit / 8-bit Off-Screen Graphics Framebuffers (SCR1..8:).
//
// ---- Includes ----

#include "device/dev_framebuffer.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/strings.h"
#include "runtime/memory/alloc.h"

#define FB_WIDTH  320
#define FB_HEIGHT 200
#define FB_SIZE   (FB_WIDTH * FB_HEIGHT)

typedef struct {
    int     index;
    char    name[16];
    int     width;
    int     height;
    int     bpp; // 4 or 8 bits per pixel (max 256 colors)
    uint8_t pixels[FB_SIZE];
    int     cursor_x;
    int     cursor_y;
    uint8_t draw_color;
} FramebufferContext;

static FramebufferContext s_fb_instances[10];
static bool s_fb_inited = false;

static void fb_init_all(void) {
    if (s_fb_inited) return;
    runtime_memset(s_fb_instances, 0, sizeof(s_fb_instances));
    for (int i = 0; i < 10; i++) {
        s_fb_instances[i].index = i;
        s_fb_instances[i].width = FB_WIDTH;
        s_fb_instances[i].height = FB_HEIGHT;
        s_fb_instances[i].bpp = 8;
        s_fb_instances[i].draw_color = 15;
        runtime_snprintf(s_fb_instances[i].name, sizeof(s_fb_instances[i].name), "SCR%d:", i);
    }
    s_fb_inited = true;
}

void dev_fb_pset(int buffer_index, int x, int y, uint8_t color) {
    fb_init_all();
    int idx = (buffer_index >= 0 && buffer_index <= 9) ? buffer_index : 1;
    FramebufferContext *ctx = &s_fb_instances[idx];
    if (x >= 0 && x < ctx->width && y >= 0 && y < ctx->height) {
        ctx->pixels[y * ctx->width + x] = color;
    }
}

uint8_t dev_fb_pget(int buffer_index, int x, int y) {
    fb_init_all();
    int idx = (buffer_index >= 0 && buffer_index <= 9) ? buffer_index : 1;
    FramebufferContext *ctx = &s_fb_instances[idx];
    if (x >= 0 && x < ctx->width && y >= 0 && y < ctx->height) {
        return ctx->pixels[y * ctx->width + x];
    }
    return 0;
}

void dev_fb_blit(int dst_buf, int dst_x, int dst_y, int src_buf, int src_x, int src_y, int w, int h) {
    fb_init_all();
    int s_idx = (src_buf >= 0 && src_buf <= 9) ? src_buf : 1;
    int d_idx = (dst_buf >= 0 && dst_buf <= 9) ? dst_buf : 1;
    FramebufferContext *s = &s_fb_instances[s_idx];
    FramebufferContext *d = &s_fb_instances[d_idx];

    for (int row = 0; row < h; row++) {
        int sy = src_y + row;
        int dy = dst_y + row;
        if (sy < 0 || sy >= s->height || dy < 0 || dy >= d->height) continue;
        for (int col = 0; col < w; col++) {
            int sx = src_x + col;
            int dx = dst_x + col;
            if (sx < 0 || sx >= s->width || dx < 0 || dx >= d->width) continue;
            d->pixels[dy * d->width + dx] = s->pixels[sy * s->width + sx];
        }
    }
}

static int fb_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    FramebufferContext *ctx = (FramebufferContext *)dev->priv;
    if (c == '\n') {
        ctx->cursor_x = 0;
        ctx->cursor_y += 8;
        if (ctx->cursor_y >= ctx->height) ctx->cursor_y = 0;
        return c;
    }
    if (c >= 32 && c <= 126) {
        // Draw simple 8x8 block for char
        for (int r = 0; r < 8; r++) {
            for (int col = 0; col < 8; col++) {
                dev_fb_pset(ctx->index, ctx->cursor_x + col, ctx->cursor_y + r, ctx->draw_color);
            }
        }
        ctx->cursor_x += 8;
        if (ctx->cursor_x >= ctx->width) {
            ctx->cursor_x = 0;
            ctx->cursor_y += 8;
            if (ctx->cursor_y >= ctx->height) ctx->cursor_y = 0;
        }
    }
    return c;
}

static int fb_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    int count = 0;
    while (*s) {
        fb_putc(dev, (int)(unsigned char)*s++);
        count++;
    }
    return count;
}

static int fb_cls(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    FramebufferContext *ctx = (FramebufferContext *)dev->priv;
    runtime_memset(ctx->pixels, 0, sizeof(ctx->pixels));
    ctx->cursor_x = 0;
    ctx->cursor_y = 0;
    return 0;
}

static int fb_dev_ioctl(VDev *d, int cmd, void *arg) {
    if (!d || !d->priv) return -1;
    FramebufferContext *ctx = (FramebufferContext *)d->priv;
    switch (cmd) {
        case 0: // CLS
            return fb_cls(d);
        case 40: // DRAWTO
            if (arg) {
                int *coords = (int *)arg;
                // Simple line draw from (cursor_x, cursor_y) to coords[0], coords[1]
                int x1 = ctx->cursor_x, y1 = ctx->cursor_y;
                int x2 = coords[0], y2 = coords[1];
                int dx = abs(x2 - x1), dy = abs(y2 - y1);
                int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
                int err = dx - dy;
                while (true) {
                    dev_fb_pset(ctx->index, x1, y1, ctx->draw_color);
                    if (x1 == x2 && y1 == y2) break;
                    int e2 = 2 * err;
                    if (e2 > -dy) { err -= dy; x1 += sx; }
                    if (e2 < dx) { err += dx; y1 += sy; }
                }
                ctx->cursor_x = x2;
                ctx->cursor_y = y2;
            }
            return 0;
        case 41: // FILL / RECTANGLE
            if (arg) {
                int *rect = (int *)arg; // x, y, w, h, color
                for (int r = 0; r < rect[3]; r++) {
                    for (int c = 0; c < rect[2]; c++) {
                        dev_fb_pset(ctx->index, rect[0] + c, rect[1] + r, (uint8_t)rect[4]);
                    }
                }
            }
            return 0;
        default:
            return 0;
    }
}

static const char *fb_dev_info(VDev *d, const char *key) {
    if (!d || !key) return NULL;
    if (runtime_strcasecmp(key, "CLASS") == 0) return "FRAMEBUFFER";
    if (runtime_strcasecmp(key, "DRIVER") == 0) return "dev_framebuffer";
    if (runtime_strcasecmp(key, "VERSION") == 0) return "6.5.2";
    if (runtime_strcasecmp(key, "CAPS") == 0) return "WRITE,GRAPHICS,256_COLOR,BLIT";
    return NULL;
}

VDev dev_framebuffer_create(const char *name, int buffer_index) {
    fb_init_all();
    int idx = (buffer_index >= 0 && buffer_index <= 9) ? buffer_index : 1;
    FramebufferContext *ctx = &s_fb_instances[idx];
    if (name && name[0]) {
        runtime_snprintf(ctx->name, sizeof(ctx->name), "%s", name);
    }

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = ctx->name;
    dev.dev_class = VDCLASS_FRAMEBUFFER;
    dev.dev_caps = VDCAP_WRITE | VDCAP_BINARY | VDCAP_STATUS | VDCAP_CONTROL;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "4-bit / 8-bit Off-Screen Pixel Canvas";
    dev.priv = ctx;

    dev.ops.putc = fb_putc;
    dev.ops.puts = fb_puts;
    dev.ops.getc = NULL;
    dev.ops.gets = NULL;
    dev.ops.flush = NULL;
    dev.ops.cls = fb_cls;

    dev.dev_read = NULL;
    dev.dev_write = NULL;
    dev.dev_seek = NULL;
    dev.dev_ioctl = fb_dev_ioctl;
    dev.dev_status = NULL;
    dev.dev_poll = NULL;
    dev.dev_info = fb_dev_info;
    dev.dev_open = NULL;
    dev.dev_close = NULL;

    return dev;
}
