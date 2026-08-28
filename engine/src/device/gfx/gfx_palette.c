// FILENAME: gfx_palette.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libengine (bgi.h, bgi.c)
// NEEDS: libkernel (gfx_internal.h)
// Implements virtual device and graphics rendering logic for gfx_palette.
//
// ---- Includes ----

#include "device/bgi.h"
#include "device/gfx_internal.h"
#include "runtime/string/memops.h"

BppColor g_palette[256];

//
// ---- Palette Initialization ----

// Populates the default 16-color CGA/EGA/VGA standard color palette
void init_palette(void) {
    static const BppColor std_colors[16] = {
        {0, 0, 0, 255},       // 0: Black
        {0, 0, 170, 255},     // 1: Blue
        {0, 170, 0, 255},     // 2: Green
        {0, 170, 170, 255},   // 3: Cyan
        {170, 0, 0, 255},     // 4: Red
        {170, 0, 170, 255},   // 5: Magenta
        {170, 85, 0, 255},    // 6: Brown
        {170, 170, 170, 255}, // 7: Light Gray
        {85, 85, 85, 255},    // 8: Dark Gray
        {85, 85, 255, 255},   // 9: Light Blue
        {85, 255, 85, 255},   // 10: Light Green
        {85, 255, 255, 255},  // 11: Light Cyan
        {255, 85, 85, 255},   // 12: Light Red
        {255, 85, 255, 255},  // 13: Light Magenta
        {255, 255, 85, 255},  // 14: Yellow
        {255, 255, 255, 255}  // 15: Bright White
    };
    runtime_memcpy(g_palette, std_colors, sizeof(std_colors));
}

//
// ---- BGI Synchronization ----

// Maps a BASIC SCREEN number to a BGI heritage mode identifier
uint32_t screen_mode_to_bgi_id(int screen_mode) {
    switch (screen_mode) {
        case 1:   return BGI_MODE_CGA_1;
        case 2:   return BGI_MODE_CGA_2;
        case 3:   return BGI_MODE_TANDY_3;
        case 9:   return BGI_MODE_EGA_9;
        case 10:  return BGI_MODE_EGA_10;
        case 12:  return BGI_MODE_VGA_12;
        case 13:  return BGI_MODE_VGA_13;
        default:  return 0;
    }
}

// Synchronizes the BGI framebuffer to the software pixel buffer
static void bgi_sync_to_host(void) {
    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized || !ctx->framebuffer) return;

    BGI_synthesize(ctx);
    vdev_gfx_poll_events();
}

// Initializes BGI primary renderer for the given screen mode
void bgi_sync_screen_mode(int screen_mode) {
    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized) {
        BGI_init(ctx);
    }
    uint32_t mode_id = screen_mode_to_bgi_id(screen_mode);
    if (mode_id != 0) {
        BGI_set_mode_by_id(ctx, mode_id);
    }
    bgi_set_sync_hook(bgi_sync_to_host);
}

// Shuts down BGI when returning to text mode
void bgi_shutdown_mode(void) {
    BGI_Context *ctx = BGI_get_global_context();
    if (ctx->initialized) {
        BGI_shutdown(ctx);
    }
}
