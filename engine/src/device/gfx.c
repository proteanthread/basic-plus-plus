// FILENAME: gfx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements virtual device and graphics rendering logic for gfx.
//
// ---- Includes ----

#include "device/bgi.h"
#include "device/gfx_internal.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "security/security.h"
#include "types/types.h"
#include "vm/vm.h"
#include "hal/hal.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

//
// ---- Global Hardware & Framebuffer State ----

uint32_t *g_pixels = NULL;
int       g_width = 640;
int       g_height = 480;
int       graphics_mode = 0;
int       g_grid_cols = 80;
int       g_grid_rows = 30;

bool      g_graphics_allowed = false;
bool      g_sdl_gui_boot = false;
bool      g_gfx_quit_requested = false;

//
// ---- Graphics Subsystem Activation ----

void vdev_gfx_enable(bool allowed, bool gui_boot) {
    g_graphics_allowed = allowed;
    g_sdl_gui_boot = gui_boot;
}

void vdev_gfx_boot_check(VMContext *vm) {
    if (g_graphics_allowed && g_sdl_gui_boot) {
        BppError err = vm_execute_line(vm, "SCREEN 12");
        (void)err;
    }
}

// Allocates pixel buffers and establishes window metrics using unified BGI context
BppError init_graphics_mode_dims(VMContext *vm, int width, int height) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BGI_Context *bgi = BGI_get_global_context();
    if (!bgi->initialized) {
        BGI_init(bgi);
    }

    int rc = BGI_create_custom_mode(bgi, width, height, 8, width / 8, height / 8, 60.0f);
    if (rc < 0 || !bgi->framebuffer) {
        err.code = 14;
        err.message = "Out of memory allocating software framebuffer";
        return err;
    }

    g_pixels = bgi->framebuffer;
    g_width = bgi->fb_width;
    g_height = bgi->fb_height;
    g_grid_cols = bgi->fb_width / 8;
    g_grid_rows = bgi->fb_height / 8;

    init_palette();

    if (g_graphics_allowed) {
        BGI_init_window(bgi, "BASIC++ Graphics Display");
    }

    g_cursor_x = 0;
    g_cursor_y = 0;

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) {
        con->ops.putc = gfx_con_putc;
        con->ops.puts = gfx_con_puts;
        con->ops.cls  = gfx_con_cls;
        con->ops.flush = gfx_con_flush;
        con->ops.getc = gfx_con_getc;
    }

    return err;
}

// Evaluates SCREEN statements
BppError vdev_legacy_stmt_screen_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    BValue mode_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    return vdev_legacy_stmt_screen_mode_handler(vm, (int)mode_val.as.number);
}

// Switches active display mode between console text and retro graphics
BppError vdev_legacy_stmt_screen_mode_handler(VMContext *vm, int mode) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BGI_Context *bgi = BGI_get_global_context();
    if (!bgi->initialized) {
        BGI_init(bgi);
    }

    graphics_mode = mode;
    if (mode == 0) {
        // Text mode
        BGI_shutdown_window(bgi);
        bgi_shutdown_mode();
        g_pixels = NULL;
        return err;
    }

    uint32_t bgi_id = screen_mode_to_bgi_id(mode);
    if (bgi_id == 0) {
        bgi_id = BGI_MODE_VGA_12;
    }

    int rc = BGI_set_mode_by_id(bgi, bgi_id);
    if (rc < 0) {
        err.code = 5;
        err.message = "Failed to activate screen mode in BGI engine";
        return err;
    }

    g_pixels = bgi->framebuffer;
    g_width = bgi->fb_width;
    g_height = bgi->fb_height;
    g_grid_cols = bgi->fb_width / 8;
    g_grid_rows = bgi->fb_height / 8;

    init_palette();

    if (g_graphics_allowed) {
        BGI_init_window(bgi, "BASIC++ Graphics Display");
    }

    g_cursor_x = 0;
    g_cursor_y = 0;

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) {
        con->ops.putc = gfx_con_putc;
        con->ops.puts = gfx_con_puts;
        con->ops.cls  = gfx_con_cls;
        con->ops.flush = gfx_con_flush;
        con->ops.getc = gfx_con_getc;
        con->ops.gets = gfx_con_gets;
        gfx_con_cls(con);
    }

    bgi_sync_screen_mode(mode);
    return err;
}

// Evaluates COLOR statements setting text and graphics foreground/background
BppError vdev_legacy_stmt_color_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EQ) {
        lex_next(lex);
    }

    BValue fg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int fg = (int)fg_val.as.number;

    bool has_bg = false;
    int bg = 0;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue bg_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        bg = (int)bg_val.as.number;
        has_bg = true;
    }

    if (fg >= 0 && fg < 256) g_fg_color_idx = fg;
    if (has_bg && bg >= 0 && bg < 256) g_bg_color_idx = bg;

    BGI_Context *bgi = BGI_get_global_context();
    if (bgi->initialized) {
        BGI_setcolor(bgi, g_fg_color_idx);
        BGI_setbkcolor(bgi, g_bg_color_idx);
    }

    VDevContext *vdev_ctx = vm_get_vdev(vm);
    if (vdev_ctx) {
        static const int ansi_fg_map[16] = {30,34,32,36,31,35,33,37,90,94,92,96,91,95,93,97};
        static const int ansi_bg_map[16] = {40,44,42,46,41,45,43,47,100,104,102,106,101,105,103,107};
        char esc[64] = "";
        int fg_ansi = (fg >= 0 && fg < 16) ? ansi_fg_map[fg] : fg;
        int bg_ansi = (bg >= 0 && bg < 16) ? ansi_bg_map[bg] : bg;

        if (has_bg) {
            if (fg < 16 && bg < 16) {
                runtime_snprintf(esc, sizeof(esc), "\033[%d;%dm", fg_ansi, bg_ansi);
            } else if (fg >= 16 && bg >= 16) {
                runtime_snprintf(esc, sizeof(esc), "\033[38;5;%d;48;5;%dm", fg, bg);
            } else if (fg >= 16) {
                runtime_snprintf(esc, sizeof(esc), "\033[38;5;%d;%dm", fg, bg_ansi);
            } else {
                runtime_snprintf(esc, sizeof(esc), "\033[%d;48;5;%dm", fg_ansi, bg);
            }
        } else {
            if (fg < 16) {
                runtime_snprintf(esc, sizeof(esc), "\033[%dm", fg_ansi);
            } else {
                runtime_snprintf(esc, sizeof(esc), "\033[38;5;%dm", fg);
            }
        }
        vdev_puts(vdev_ctx, esc);
    }

    return err;
}

// Present active graphics screen to window
void update_graphics_screen(void) {
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_present(bgi);
    }
}

// Poll graphics event loop
void poll_graphics_events(void) {
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized) {
        BGI_poll_events(bgi);
    }
}

// Close graphics window
void close_graphics_window(void) {
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized) {
        BGI_shutdown_window(bgi);
    }
}

//
// ---- Image Slot Memory Management (Freestanding) ----

typedef struct {
    uint32_t *pixels;
    int width;
    int height;
    bool active;
} ImageSlot;

#define MAX_IMAGE_SLOTS 256
static ImageSlot g_image_slots[MAX_IMAGE_SLOTS];

int vdev_image_create(int w, int h, int mode) {
    (void)mode;
    if (w <= 0 || h <= 0) return -1;

    int slot = -1;
    for (int i = 0; i < MAX_IMAGE_SLOTS; i++) {
        if (!g_image_slots[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    size_t num_bytes = (size_t)w * (size_t)h * sizeof(uint32_t);
    uint32_t *pixels = (uint32_t *)runtime_malloc(num_bytes);
    if (!pixels) return -1;
    runtime_memset(pixels, 0, num_bytes);

    g_image_slots[slot].pixels = pixels;
    g_image_slots[slot].width = w;
    g_image_slots[slot].height = h;
    g_image_slots[slot].active = true;
    return slot;
}

int vdev_image_load(const char *filename) {
    (void)filename;
    return -1;
}

void vdev_image_free(int handle) {
    if (handle >= 0 && handle < MAX_IMAGE_SLOTS && g_image_slots[handle].active) {
        if (g_image_slots[handle].pixels) {
            runtime_free(g_image_slots[handle].pixels);
            g_image_slots[handle].pixels = NULL;
        }
        g_image_slots[handle].active = false;
    }
}

int vdev_image_copy(int handle) {
    if (handle < 0 || handle >= MAX_IMAGE_SLOTS || !g_image_slots[handle].active) return -1;
    int slot = -1;
    for (int i = 0; i < MAX_IMAGE_SLOTS; i++) {
        if (!g_image_slots[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    int w = g_image_slots[handle].width;
    int h = g_image_slots[handle].height;
    size_t num_bytes = (size_t)w * (size_t)h * sizeof(uint32_t);
    uint32_t *pixels = (uint32_t *)runtime_malloc(num_bytes);
    if (!pixels) return -1;

    runtime_memcpy(pixels, g_image_slots[handle].pixels, num_bytes);
    g_image_slots[slot].pixels = pixels;
    g_image_slots[slot].width = w;
    g_image_slots[slot].height = h;
    g_image_slots[slot].active = true;
    return slot;
}

void vdev_image_draw(int handle, int x, int y, int w, int h) {
    if (handle < 0 || handle >= MAX_IMAGE_SLOTS || !g_image_slots[handle].active) return;
    int src_w = g_image_slots[handle].width;
    int src_h = g_image_slots[handle].height;
    uint32_t *src = g_image_slots[handle].pixels;

    if (w <= 0) w = src_w;
    if (h <= 0) h = src_h;

    BGI_Context *bgi = BGI_get_global_context();
    if (!bgi || !bgi->framebuffer) return;

    for (int dy = 0; dy < h; dy++) {
        int py = y + dy;
        if (py < 0 || py >= bgi->fb_height) continue;
        int sy = (dy * src_h) / h;

        for (int dx = 0; dx < w; dx++) {
            int px = x + dx;
            if (px < 0 || px >= bgi->fb_width) continue;
            int sx = (dx * src_w) / w;

            uint32_t pixel = src[sy * src_w + sx];
            uint8_t a = (uint8_t)((pixel >> 24) & 0xFF);
            if (a == 0) continue;

            if (a == 255) {
                bgi->framebuffer[py * bgi->fb_width + px] = pixel;
            } else {
                uint32_t dst_pixel = bgi->framebuffer[py * bgi->fb_width + px];
                uint8_t dst_r = (uint8_t)((dst_pixel >> 16) & 0xFF);
                uint8_t dst_g = (uint8_t)((dst_pixel >> 8) & 0xFF);
                uint8_t dst_b = (uint8_t)(dst_pixel & 0xFF);

                uint8_t src_r = (uint8_t)((pixel >> 16) & 0xFF);
                uint8_t src_g = (uint8_t)((pixel >> 8) & 0xFF);
                uint8_t src_b = (uint8_t)(pixel & 0xFF);

                uint8_t out_r = (uint8_t)((src_r * a + dst_r * (255 - a)) / 255);
                uint8_t out_g = (uint8_t)((src_g * a + dst_g * (255 - a)) / 255);
                uint8_t out_b = (uint8_t)((src_b * a + dst_b * (255 - a)) / 255);

                bgi->framebuffer[py * bgi->fb_width + px] = (255u << 24) | ((uint32_t)out_r << 16) | ((uint32_t)out_g << 8) | (uint32_t)out_b;
            }
        }
    }
}

int vdev_image_width(int handle) {
    if (handle >= 0 && handle < MAX_IMAGE_SLOTS && g_image_slots[handle].active) {
        return g_image_slots[handle].width;
    }
    return 0;
}

int vdev_image_height(int handle) {
    if (handle >= 0 && handle < MAX_IMAGE_SLOTS && g_image_slots[handle].active) {
        return g_image_slots[handle].height;
    }
    return 0;
}

void vdev_image_free_all(void) {
    for (int i = 0; i < MAX_IMAGE_SLOTS; i++) {
        vdev_image_free(i);
    }
}
