/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bgi_modes.c
 * @brief BGI Heritage Mode Library — Pre-registered video mode descriptors.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Registers all pre-defined heritage video modes into the
 *   BGI mode table. Each mode corresponds to a real historical graphics
 *   adapter, home computer, or game console display system.
 * - Why it exists: Allows BASIC++ programs to activate any pre-SVGA display
 *   mode with a single SCREEN or INITGRAPH command.
 * - Why it works this way: Static const mode descriptors are registered via
 *   BGI_register_mode() during BGI_init(). No VRAM is allocated until a
 *   mode is actually activated with BGI_set_mode().
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Add new mode descriptors for additional hardware.
 * - What cannot be changed: Existing mode_id values (backward compatibility).
 * - What to expect: All modes use the default 16-color palette unless they
 *   override it with a custom palette pointer.
 * - What to do if something breaks: Verify mode_id uniqueness, verify
 *   width/height > 0, verify bits_per_pixel is valid.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: BGI_Context is initialized before this is called.
 * - Portability concerns: Pure ISO C17. No OS dependencies.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add new entries to the modes array.
 * - How to write external extensions: Call BGI_register_mode() at runtime.
 */

#include "device/bgi.h"
#include <string.h>

/* ======================================================================
 * Helper macro to define a mode descriptor inline
 * ====================================================================== */
#define BGI_DEF_MODE(name_str, id, w, h, bpp, pal_sz, layout, ar) \
    { name_str, id, w, h, bpp, pal_sz, NULL, layout, 0, 0, ar, 60.0f }

/* ======================================================================
 * Heritage Mode Descriptor Table
 * ====================================================================== */
static const BGI_VideoMode bgi_heritage_modes[] = {
    /* === IBM PC Graphics Adapters === */
    BGI_DEF_MODE("CGA SCREEN 1",     BGI_MODE_CGA_1,    320, 200,  2,   4,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("CGA SCREEN 2",     BGI_MODE_CGA_2,    640, 200,  1,   2,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Tandy SCREEN 3",   BGI_MODE_TANDY_3,  160, 200,  4,  16,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("EGA SCREEN 9",     BGI_MODE_EGA_9,    640, 350,  4,  16,  BGI_LAYOUT_PLANAR_4PLANE,  4.0f/3.0f),
    BGI_DEF_MODE("EGA SCREEN 10",    BGI_MODE_EGA_10,   640, 350,  1,   2,  BGI_LAYOUT_PLANAR_4PLANE,  4.0f/3.0f),
    BGI_DEF_MODE("VGA SCREEN 12",    BGI_MODE_VGA_12,   640, 480,  4,  16,  BGI_LAYOUT_PLANAR_4PLANE,  4.0f/3.0f),
    BGI_DEF_MODE("VGA Mode 13h",     BGI_MODE_VGA_13,   320, 200,  8, 256,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Hercules HGC",     BGI_MODE_HGC,      720, 348,  1,   2,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("IBM MDA",          BGI_MODE_MDA,      720, 350,  1,   2,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),

    /* === Home Computers === */
    BGI_DEF_MODE("C64 Hi-Res",       BGI_MODE_C64_HI,   320, 200,  4,  16,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("C64 Multicolor",   BGI_MODE_C64_MC,   160, 200,  2,   4,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("ZX Spectrum",      BGI_MODE_ZX_SPEC,  256, 192,  1,  16,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("Atari 8-bit GR.8", BGI_MODE_ATARI8_8, 320, 192,  1,   2,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Atari 8-bit GR.7", BGI_MODE_ATARI8_7, 160,  96,  2,   4,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Atari ST Low",     BGI_MODE_ATARI_ST_L,320,200,  4,  16,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Atari ST Medium",  BGI_MODE_ATARI_ST_M,640,200,  2,   4,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Atari ST High",    BGI_MODE_ATARI_ST_H,640,400,  1,   2,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Apple II Hi-Res",  BGI_MODE_APPLE2_HI, 280,192,  3,   6,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Amiga OCS 320",    BGI_MODE_AMIGA_320, 320,256,  5,  32,  BGI_LAYOUT_PLANAR_4PLANE,  4.0f/3.0f),
    BGI_DEF_MODE("Amiga OCS 640",    BGI_MODE_AMIGA_640, 640,256,  4,  16,  BGI_LAYOUT_PLANAR_4PLANE,  4.0f/3.0f),
    BGI_DEF_MODE("MSX1 TMS9918",     BGI_MODE_MSX1,     256, 212,  4,  16,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("Amstrad CPC M0",   BGI_MODE_AMSTRAD_0, 160,200,  4,  16,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("BBC Micro Mode 1", BGI_MODE_BBC_M1,   160, 256,  3,   8,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),

    /* === Game Consoles === */
    BGI_DEF_MODE("NES PPU NTSC",     BGI_MODE_NES_NTSC, 256, 240,  6,  54,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("SNES PPU",         BGI_MODE_SNES,     256, 224, 15, 256,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Genesis NTSC",     BGI_MODE_GENESIS_N, 320,224,  9, 256,  BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Master System",    BGI_MODE_SMS_NTSC, 256, 192,  6,  64,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("Game Boy Color",   BGI_MODE_GBC,      160, 144,  6,  56,  BGI_LAYOUT_INDEXED_8BPP,   10.0f/9.0f),
    BGI_DEF_MODE("Game Gear",        BGI_MODE_GG,       160, 144,  5,  32,  BGI_LAYOUT_INDEXED_8BPP,   10.0f/9.0f),
    BGI_DEF_MODE("TurboGrafx-16",    BGI_MODE_TG16,     256, 239,  9, 256,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("Intellivision",    BGI_MODE_INTELLI,  159,  96,  4,  16,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("ColecoVision",     BGI_MODE_COLECO,   256, 192,  4,  16,  BGI_LAYOUT_CELL_ATTRIBUTES,4.0f/3.0f),
    BGI_DEF_MODE("Atari 2600 TIA",   BGI_MODE_ATARI_2600,160,192,  7, 128, BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Atari 7800 MARIA", BGI_MODE_ATARI_7800,160,240,  8, 256, BGI_LAYOUT_INDEXED_8BPP,   4.0f/3.0f),
    BGI_DEF_MODE("Game Boy",         BGI_MODE_GB,       160, 144,  2,   4,  BGI_LAYOUT_INDEXED_8BPP,   10.0f/9.0f),
};

#define BGI_NUM_HERITAGE_MODES \
    (int)(sizeof(bgi_heritage_modes) / sizeof(bgi_heritage_modes[0]))

/* ======================================================================
 * Registration Function
 * ====================================================================== */

void BGI_register_heritage_modes(BGI_Context *ctx) {
    if (!ctx) return;

    for (int i = 0; i < BGI_NUM_HERITAGE_MODES; ++i) {
        BGI_register_mode(ctx, &bgi_heritage_modes[i]);
    }
}
