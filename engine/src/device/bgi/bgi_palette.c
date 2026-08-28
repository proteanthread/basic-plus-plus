// FILENAME: bgi_palette.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (context.c)
// NEEDED BY: libhardware (bios_pc.c)
// NEEDED BY: libkernel (bgi_gfx.h, bgi_gfx.c)
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libkernel (bgi_palette.h)
// Implements virtual device and graphics rendering logic for bgi_palette.
//
// ---- Includes ----

#include "device/bgi_palette.h"
#include "runtime/string/memops.h"

static BgiPalette g_global_palette;

// Standard IBM 16-color CGA/EGA Palette (ARGB8888)
static const uint32_t s_default_16_colors[16] = {
    0xFF000000U, // 0: Black
    0xFF0000AAU, // 1: Blue
    0xFF00AA00U, // 2: Green
    0xFF00AAAAU, // 3: Cyan
    0xFFAA0000U, // 4: Red
    0xFFAA00AAU, // 5: Magenta
    0xFFAA5500U, // 6: Brown
    0xFFAAAAAAU, // 7: Light Gray
    0xFF555555U, // 8: Dark Gray
    0xFF5555FFU, // 9: Light Blue
    0xFF55FF55U, // 10: Light Green
    0xFF55FFFFU, // 11: Light Cyan
    0xFFFF5555U, // 12: Light Red
    0xFFFF55FFU, // 13: Light Magenta
    0xFFFFFF55U, // 14: Yellow
    0xFFFFFFFFU  // 15: White
};

void bgi_palette_init(BgiPalette *pal) {
    if (!pal) return;
    runtime_memset(pal, 0, sizeof(*pal));


    // Initialize first 16 entries with standard CGA/EGA colors
    for (int i = 0; i < 16; ++i) {
        pal->colors[i] = s_default_16_colors[i];
        pal->dac_r[i] = (uint8_t)(((s_default_16_colors[i] >> 16) & 0xFF) * 63 / 255);
        pal->dac_g[i] = (uint8_t)(((s_default_16_colors[i] >>  8) & 0xFF) * 63 / 255);
        pal->dac_b[i] = (uint8_t)(((s_default_16_colors[i]       & 0xFF) * 63) / 255);
    }

    // Initialize VGA Mode 13h 256-color palette ramp (entries 16-255)
    // 6x6x6 color cube for 16-215, 24 grayscale levels for 216-255
    for (int i = 16; i < 256; ++i) {
        uint8_t r = 0, g = 0, b = 0;
        if (i < 216) {
            int idx = i - 16;
            r = (uint8_t)((idx / 36) * 51);
            g = (uint8_t)(((idx / 6) % 6) * 51);
            b = (uint8_t)((idx % 6) * 51);
        } else {
            uint8_t gray = (uint8_t)((i - 216) * 10 + 8);
            r = gray; g = gray; b = gray;
        }
        pal->colors[i] = (0xFF000000U) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
        pal->dac_r[i] = (uint8_t)(r * 63 / 255);
        pal->dac_g[i] = (uint8_t)(g * 63 / 255);
        pal->dac_b[i] = (uint8_t)(b * 63 / 255);
    }

    pal->num_colors = 256;
    pal->initialized = true;
}

void bgi_palette_set_color(BgiPalette *pal, uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (!pal) return;
    pal->colors[index] = (0xFF000000U) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    pal->dac_r[index] = (uint8_t)(r * 63 / 255);
    pal->dac_g[index] = (uint8_t)(g * 63 / 255);
    pal->dac_b[index] = (uint8_t)(b * 63 / 255);
}

void bgi_palette_set_dac(BgiPalette *pal, uint8_t index, uint8_t r6, uint8_t g6, uint8_t b6) {
    if (!pal) return;
    uint8_t r8 = (uint8_t)((r6 & 0x3F) * 255 / 63);
    uint8_t g8 = (uint8_t)((g6 & 0x3F) * 255 / 63);
    uint8_t b8 = (uint8_t)((b6 & 0x3F) * 255 / 63);
    pal->colors[index] = (0xFF000000U) | ((uint32_t)r8 << 16) | ((uint32_t)g8 << 8) | (uint32_t)b8;
    pal->dac_r[index] = r6 & 0x3F;
    pal->dac_g[index] = g6 & 0x3F;
    pal->dac_b[index] = b6 & 0x3F;
}

uint32_t bgi_palette_get_argb(const BgiPalette *pal, uint8_t index) {
    if (!pal) return 0xFF000000U;
    return pal->colors[index];
}

void bgi_palette_handle_port_write(BgiPalette *pal, uint16_t port, uint8_t val) {
    if (!pal) return;
    if (port == 0x03C8) {
        // Set PEL write index & reset R/G/B write sequence to Red step
        pal->dac_state.write_index = val;
        pal->dac_state.rgb_seq = 0;
    } else if (port == 0x03C9) {
        // Process sequential R, G, B bytes
        switch (pal->dac_state.rgb_seq) {
            case 0:
                pal->dac_state.pending_r = val & 0x3F;
                pal->dac_state.rgb_seq = 1;
                break;
            case 1:
                pal->dac_state.pending_g = val & 0x3F;
                pal->dac_state.rgb_seq = 2;
                break;
            case 2: {
                uint8_t b6 = val & 0x3F;
                bgi_palette_set_dac(pal, pal->dac_state.write_index,
                                    pal->dac_state.pending_r,
                                    pal->dac_state.pending_g,
                                    b6);
                pal->dac_state.write_index++; // Auto-increment PEL index
                pal->dac_state.rgb_seq = 0;   // Reset to Red step for next color
                break;
            }
        }
    }
}

BgiPalette* bgi_palette_get_global(void) {
    if (!g_global_palette.initialized) {
        bgi_palette_init(&g_global_palette);
    }
    return &g_global_palette;
}
