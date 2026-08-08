/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bus.c
 * @brief Virtual hardware bus multiplexer and device registration router for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `vdev_bus_init()`, `vdev_bus_register()`, `vdev_bus_in()`, `vdev_bus_out()`, `vdev_bus_peek()`, `vdev_bus_poke()`.
 *
 * 2. WHY IT EXISTS:
 * Provides virtual hardware I/O bus routing (0x0000..0xFFFF ports) connecting hardware virtual devices (COM, LPT, Sound, Video, Network) to BASIC statements (`INP`, `OUT`, `PEEK`, `POKE`).
 *
 * 3. WHY IT WORKS THIS WAY:
 * Maintains a array of registered virtual devices (`VDevBusDevice`), matching port ranges and segment offsets to dispatch device-specific read/write callbacks.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "device/bus.h", "device/vdev.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Register new virtual bus devices (`vdev_bus_register`) for custom extensions.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Port address bounds (16-bit 0x0000..0xFFFF) and 8-bit bus data width.
 *
 * 8. WHAT TO EXPECT:
 * `vdev_bus_in()` returns byte value (0..255) from target device or 0xFF if unmapped port.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check virtual device port overlap conflicts in `vdev_bus_register()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized bus context.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Virtualized bus avoids host platform assembly dependencies.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/device/vdev.c
 * Prerequisite Header Files:
 * - engine/include/device/bus.h
 * - engine/include/device/vdev.h
 */

/**
 * @file vdev_bus.c
 * @brief Virtual Hardware Port & MMIO Bus implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements routing logic for INP/OUT port calls and maps CGA text RAM
 *   and BIOS Data Area PEEK/POKE writes directly to virtual console frames.
 * - Why it exists: Provides dynamic virtual hardware interfaces for the Mock BIOS.
 * - Why it works this way: It intercepts address ranges (0x400 for BDA, 0xB8000 for CGA)
 *   and redirects them to virtual buffers.
 */

#include "device/bus.h"
#include "device/vcon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PORT_HANDLERS 16

#ifndef NO_SDL2
extern void vdev_play_sound_freq(double freq, double duration_seconds);
#endif

typedef struct {
    int start;
    int end;
    int (*read_fn)(int);
    void (*write_fn)(int, int);
    bool active;
} PortHandlerEntry;

static PortHandlerEntry handlers[MAX_PORT_HANDLERS];
static uint8_t bda_ram[256]; /* Mock BIOS Data Area RAM */
static uint8_t cga_ram[4000]; /* Mock CGA 80x25 Text screen (2000 chars + 2000 attribs) */
static uint8_t c64_regs[4];  /* C64 registers at 780-783: A, X, Y, Status */
static uint8_t sid_regs[32];  /* C64 SID registers at 54272-54296 */

static MockBiosModel active_model = BIOS_MODEL_NONE;
static uint8_t *g_bios_ram = NULL;
static size_t g_bios_ram_size = 0;

void vdev_bus_set_model(MockBiosModel model) {
    active_model = model;
}

MockBiosModel vdev_bus_get_model(void) {
    return active_model;
}

void vdev_bus_set_ram(uint8_t *ram, size_t size) {
    g_bios_ram = ram;
    g_bios_ram_size = size;
}

void vdev_bus_reset(void) {
    active_model = BIOS_MODEL_NONE;
    g_bios_ram = NULL;
    g_bios_ram_size = 0;
    memset(handlers, 0, sizeof(handlers));
    memset(bda_ram, 0, sizeof(bda_ram));
    memset(cga_ram, 0, sizeof(cga_ram));
    memset(c64_regs, 0, sizeof(c64_regs));
    memset(sid_regs, 0, sizeof(sid_regs));

    /* Pre-fill BDA with default IBM PC base settings */
    bda_ram[0x00] = 0xF8; bda_ram[0x01] = 0x03; /* COM1: 0x03F8 */
    bda_ram[0x02] = 0xF8; bda_ram[0x03] = 0x02; /* COM2: 0x02F8 */
    bda_ram[0x08] = 0x78; bda_ram[0x09] = 0x03; /* LPT1: 0x0378 */
    bda_ram[0x0A] = 0x78; bda_ram[0x0B] = 0x02; /* LPT2: 0x0278 */

    /* Equipment Word: 0x022D (1 floppy, 80x25 color, 2 serial, 1 printer) */
    bda_ram[0x10] = 0x2D;
    bda_ram[0x11] = 0x02;

    /* Conventional memory size: 640KB at 0x413 */
    bda_ram[0x13] = 0x80;
    bda_ram[0x14] = 0x02;

    /* Video state */
    bda_ram[0x49] = 0x03; /* Video mode 3 */
    bda_ram[0x4A] = 80;   /* 80 columns */
    bda_ram[0x4B] = 0x00;
    bda_ram[0x63] = 0xD4; bda_ram[0x64] = 0x03; /* CRT base: 0x03D4 */
}

bool vdev_bus_register_port(int start, int end, int (*read_fn)(int), void (*write_fn)(int, int)) {
    for (int i = 0; i < MAX_PORT_HANDLERS; ++i) {
        if (!handlers[i].active) {
            handlers[i].start = start;
            handlers[i].end = end;
            handlers[i].read_fn = read_fn;
            handlers[i].write_fn = write_fn;
            handlers[i].active = true;
            return true;
        }
    }
    return false;
}

void vdev_bus_out(int port, int value) {
    /* Tandy SN76496 Sound Port $00C0 (192) */
    if (active_model == BIOS_MODEL_TANDY && port == 0xC0) {
        if (value > 0) {
#ifndef NO_SDL2
            vdev_play_sound_freq(440.0, 0.2);
#endif
        }
    }

    for (int i = 0; i < MAX_PORT_HANDLERS; ++i) {
        if (handlers[i].active && port >= handlers[i].start && port <= handlers[i].end) {
            if (handlers[i].write_fn) {
                handlers[i].write_fn(port, value);
                return;
            }
        }
    }
}

int vdev_bus_in(int port) {
    for (int i = 0; i < MAX_PORT_HANDLERS; ++i) {
        if (handlers[i].active && port >= handlers[i].start && port <= handlers[i].end) {
            if (handlers[i].read_fn) {
                return handlers[i].read_fn(port);
            }
        }
    }
    return 0xFF;
}

uint8_t vdev_bus_peek(unsigned long addr, bool *intercepted) {
    if (intercepted) *intercepted = false;

    uint8_t val = 0;
    if (g_bios_ram && addr < g_bios_ram_size) {
        val = g_bios_ram[addr];
    }

    /* 1. ATARI POKEY Random Number Generator ($D20A / 53770) */
    if (active_model == BIOS_MODEL_ATARI && addr == 53770) {
        if (intercepted) *intercepted = true;
        return (uint8_t)(rand() & 0xFF);
    }

    /* 2. APPLE II Keyboard Latch ($C000 / 49152) */
    if (active_model == BIOS_MODEL_APPLE2 && addr == 49152) {
        if (intercepted) *intercepted = true;
        return 0;
    }

    /* 3. APPLE II Speaker click ($C030 / 49184) */
    if (active_model == BIOS_MODEL_APPLE2 && addr == 49184) {
        if (intercepted) *intercepted = true;
#ifndef NO_SDL2
        vdev_play_sound_freq(440.0, 0.05);
#endif
        return 0;
    }

    /* 4. Spectrum Frame Counter Clock: 23672 - 23674 (50 Hz) */
    if (addr >= 23672 && addr <= 23674) {
        if (intercepted) *intercepted = true;
        double sec = (double)clock() / (double)CLOCKS_PER_SEC;
        uint32_t ticks = (uint32_t)(sec * 50.0);
        unsigned long offset = addr - 23672;
        return (uint8_t)((ticks >> (offset * 8)) & 0xFF);
    }

    /* 5. C64 Clock: 160 - 162 (60 Hz) */
    if (addr >= 160 && addr <= 162) {
        if (intercepted) *intercepted = true;
        double sec = (double)clock() / (double)CLOCKS_PER_SEC;
        uint32_t ticks = (uint32_t)(sec * 60.0);
        unsigned long offset = 162 - addr; /* 162 is LSB, 160 is MSB */
        return (uint8_t)((ticks >> (offset * 8)) & 0xFF);
    }

    /* Standard fallback intercepts when no bios_ram is linked */
    if (!g_bios_ram) {
        if (addr >= 0x400 && addr <= 0x4FF) {
            if (intercepted) *intercepted = true;
            return bda_ram[addr - 0x400];
        }
        if (addr >= 0xB8000 && addr <= 0xB8FA0) {
            if (intercepted) *intercepted = true;
            return cga_ram[addr - 0xB8000];
        }
        if (addr >= 780 && addr <= 783) {
            if (intercepted) *intercepted = true;
            return c64_regs[addr - 780];
        }
        if (addr >= 54272 && addr <= 54296) {
            if (intercepted) *intercepted = true;
            return sid_regs[addr - 54272];
        }
    }

    return val;
}

void vdev_bus_poke(unsigned long addr, uint8_t value, bool *intercepted) {
    if (intercepted) *intercepted = false;

    if (g_bios_ram && addr < g_bios_ram_size) {
        g_bios_ram[addr] = value;
    }

    if (!g_bios_ram) {
        if (addr >= 0x400 && addr <= 0x4FF) {
            if (intercepted) *intercepted = true;
            bda_ram[addr - 0x400] = value;
        }
        if (addr >= 0xB8000 && addr <= 0xB8FA0) {
            if (intercepted) *intercepted = true;
            cga_ram[addr - 0xB8000] = value;
        }
        if (addr >= 780 && addr <= 783) {
            if (intercepted) *intercepted = true;
            c64_regs[addr - 780] = value;
        }
    }

    /* 1. C64/VIC20 SID Sound Synthesizer ($D400 - $D41C / 54272 - 54296) */
    if ((active_model == BIOS_MODEL_C64 || active_model == BIOS_MODEL_VIC20) && (addr >= 54272 && addr <= 54296)) {
        if (intercepted) *intercepted = true;
        unsigned long offset = addr - 54272;
        sid_regs[offset] = value;
        if (offset == 4 || offset == 11 || offset == 18) {
            if (value & 0x01) {
                int base = (offset == 4) ? 0 : ((offset == 11) ? 7 : 14);
                uint16_t freq_reg = (sid_regs[base + 1] << 8) | sid_regs[base];
                double freq = (double)freq_reg * 0.06097;
                if (freq > 20.0 && freq < 20000.0) {
#ifndef NO_SDL2
                    vdev_play_sound_freq(freq, 0.5);
#endif
                }
            }
        }
    }

    /* 2. VIC-20 Audio Generators ($900A - $900D / 36874 - 36877) */
    if (active_model == BIOS_MODEL_VIC20 && (addr >= 36874 && addr <= 36877)) {
        if (intercepted) *intercepted = true;
        if (value > 0) {
            double freq = 3995000.0 / (256.0 - value);
            if (freq > 20.0 && freq < 20000.0) {
#ifndef NO_SDL2
                vdev_play_sound_freq(freq, 0.3);
#endif
            }
        }
    }

    /* 3. ATARI POKEY Audio Channels ($D200 - $D207 / 53760 - 53767) */
    if (active_model == BIOS_MODEL_ATARI && (addr >= 53760 && addr <= 53767)) {
        if (intercepted) *intercepted = true;
        int is_freq = ((addr - 53760) % 2 == 0);
        if (is_freq && value > 0) {
            double freq = 64000.0 / (value + 1);
            if (freq > 20.0 && freq < 20000.0) {
#ifndef NO_SDL2
                vdev_play_sound_freq(freq, 0.4);
#endif
            }
        }
    }

    /* 4. APPLE II Speaker toggle ($C030 / 49184) */
    if (active_model == BIOS_MODEL_APPLE2 && addr == 49184) {
        if (intercepted) *intercepted = true;
#ifndef NO_SDL2
        vdev_play_sound_freq(440.0, 0.05);
#endif
    }
}
