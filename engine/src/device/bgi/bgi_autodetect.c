// FILENAME: bgi_autodetect.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (context.c, screen.c)
// NEEDS: libcore (memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libkernel (bgi_autodetect.h)
// Implements virtual device and graphics rendering logic for bgi_autodetect.
//
// ---- Includes ----

#include "device/bgi_autodetect.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"

typedef struct ExtModeDef {
    const char *name;
    uint32_t    id;
    uint32_t    w;
    uint32_t    h;
    uint8_t     bpp;
} ExtModeDef;

static const ExtModeDef s_extended_modes[] = {
    { "SVGA SCREEN 14",  14,  800,  600, 32 },
    { "XGA SCREEN 15",   15, 1024,  768, 32 },
    { "720p SCREEN 16",  16, 1280,  720, 32 },
    { "1080p SCREEN 17", 17, 1920, 1080, 32 },
    { "1440p SCREEN 18", 18, 2560, 1440, 32 },
    { "4K SCREEN 19",    19, 3840, 2160, 32 }
};

#define NUM_EXTENDED_MODES (int)(sizeof(s_extended_modes) / sizeof(s_extended_modes[0]))

int bgi_autodetect_modes(BGI_Context *ctx) {
    if (!ctx) return 0;
    int registered = 0;

    for (int i = 0; i < NUM_EXTENDED_MODES; ++i) {
        BGI_VideoMode mode;
        runtime_memset(&mode, 0, sizeof(mode));
        runtime_snprintf(mode.mode_name, sizeof(mode.mode_name), "%s", s_extended_modes[i].name);
        mode.mode_id = s_extended_modes[i].id;
        mode.width = s_extended_modes[i].w;
        mode.height = s_extended_modes[i].h;
        mode.bits_per_pixel = s_extended_modes[i].bpp;
        mode.palette_size = 0; // Direct ARGB8888
        mode.mem_layout = BGI_LAYOUT_LINEAR_ARGB8888;
        mode.aspect_ratio = (float)s_extended_modes[i].w / (float)s_extended_modes[i].h;
        mode.refresh_hz = 60.0f;

        if (BGI_register_mode(ctx, &mode)) {
            registered++;
        }
    }

    return registered;
}

int bgi_get_detected_mode_count(void) {
    return NUM_EXTENDED_MODES;
}

void bgi_format_mode_info(int mode_idx, char *buf, size_t buf_size) {
    if (!buf || buf_size == 0) return;
    if (mode_idx < 0 || mode_idx >= NUM_EXTENDED_MODES) {
        runtime_snprintf(buf, buf_size, "Unknown Mode");
        return;
    }
    const ExtModeDef *m = &s_extended_modes[mode_idx];
    runtime_snprintf(buf, buf_size, "SCREEN %u: %s (%ux%u, %u-bit Truecolor)",
             (unsigned)m->id, m->name, (unsigned)m->w, (unsigned)m->h, (unsigned)m->bpp);
}

