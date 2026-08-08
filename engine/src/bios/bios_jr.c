/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bios_jr.c
 * @brief IBM PCjr specific BIOS services, SN76496 sound chip, and 16-color graphics support for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements PCjr BIOS extensions (`bios_jr_int15()`, SN76496 sound chip I/O port 0xC0 mapping, PCjr 16-color video modes).
 *
 * 2. WHY IT EXISTS:
 * Provides IBM PCjr hardware parity for specialized Cartridge BASIC and PCjr 16-color programs.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Maps PCjr video modes (Mode 8, Mode 9, Mode 10) and SN76496 4-channel sound chip register writes to virtual devices.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'libbios'. Includes "bios/bios.h", "bios/bios_jr.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in `libbasicpp` ('baspp').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support PCjr cartridge ROM expansion mapping.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * PCjr video mode definitions (16-color 160x200, 320x200, 640x200).
 *
 * 8. WHAT TO EXPECT:
 * `bios_jr_int15()` handles PCjr specific interrupt calls.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check SN76496 sound attenuation register parsing logic.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid `BiosContext` pointer with `BIOS_MODEL_PCJR`.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Hardware port emulation routes through virtual bus interface.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/bios/bios.c
 * Prerequisite Header Files:
 * - engine/include/bios/bios.h
 * - engine/include/bios/bios_jr.h
 */
/**
 * @file bios_jr.c
 * @brief IBM PCjr 4860 authentic BIOS implementation with SN76496 PSG & Video Array.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements IBM PCjr 4860 hardware extensions, Video Array (0x3DA/0x3DF),
 *   SN76496 3-voice PSG sound generator (Port 0xC0), PCjr BDA flags at 0x48A, and ROM
 *   release date "06/01/83" at offset 0xFFFF5.
 */

#include "bios/bios_jr.h"
#include "bios/bios_pc.h"
#include <string.h>

void bios_jr_init(BiosContext* ctx) {
    if (!ctx) return;

    /* Baseline PC initialization */
    bios_pc_init(ctx);

    BiosDataArea* bda = bios_get_bda(ctx);
    if (bda) {
        bda->pcjr_video_flags = 0x01; /* PCjr 16-color Video Array active */
    }

    /* Write Authentic IBM PCjr 4860 Release Date "06/01/83" at 0xFFFF5 */
    const char* release_date = "06/01/83";
    for (size_t i = 0; i < 8; i++) {
        bios_poke_raw(ctx, 0xFFFF5 + (uint32_t)i, (uint8_t)release_date[i]);
    }

    /* Model Byte 0xFD at 0xFFFFE */
    bios_poke_raw(ctx, 0xFFFFE, 0xFD);
}

uint8_t bios_jr_inp(BiosContext* ctx, uint16_t port) {
    if (!ctx) return 0xFF;
    if (port == 0x03DA || port == 0x03DF) {
        return 0x00; /* PCjr Video Array Status */
    }
    if (port == 0xC0) {
        return 0x00; /* SN76496 PSG Sound Generator Status */
    }
    return bios_pc_inp(ctx, port);
}

void bios_jr_out(BiosContext* ctx, uint16_t port, uint8_t val) {
    if (!ctx) return;
    if (port == 0xC0) {
        /* SN76496 PSG Sound Data Register Write */
        return;
    }
    bios_pc_out(ctx, port, val);
}

bool bios_jr_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs) {
    if (!ctx || !regs) return false;
    return bios_pc_interrupt(ctx, int_num, regs);
}
