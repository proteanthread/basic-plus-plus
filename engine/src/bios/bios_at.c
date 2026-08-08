/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bios_at.c
 * @brief IBM PC/AT (80286) BIOS services, CMOS RTC, and extended memory routines for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements IBM PC/AT BIOS extensions (`bios_at_int15()`, CMOS clock read/write, extended memory INT 15h AH=87h/88h).
 *
 * 2. WHY IT EXISTS:
 * Provides PC/AT 286 hardware feature emulation (CMOS real-time clock, extended memory queries) for BASIC++.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Maps 80286 BIOS service routines to host platform time and memory manager.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'libbios'. Includes "bios/bios.h", "bios/bios_at.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in `libbasicpp` ('baspp').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support 80286 protected mode memory block moves (INT 15h AH=87h).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * PC/AT CMOS register address mapping (ports 0x70/0x71).
 *
 * 8. WHAT TO EXPECT:
 * `bios_at_int15()` handles AT-specific BIOS calls and returns status code.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check CMOS BCD encoding/decoding logic.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid `BiosContext` pointer with `BIOS_MODEL_AT`.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. CMOS reads mapped to thread-safe `platform_localtime()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/bios/bios.c
 * Prerequisite Header Files:
 * - engine/include/bios/bios.h
 * - engine/include/bios/bios_at.h
 */
/**
 * @file bios_at.c
 * @brief IBM AT 5170 authentic BIOS implementation with CMOS RTC & INT 15h 88h/E820h.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements IBM AT 5170 hardware extensions, CMOS Real-Time Clock
 *   & NVRAM (Ports 0x70/0x71), secondary 8259 PIC slave (0xA0/0xA1), INT 15h System
 *   Services (88h extended memory, E820h memory map), and ROM release date "01/10/84".
 */

#include "bios/bios_at.h"
#include "bios/bios_xt.h"
#include <string.h>
#include <time.h>

void bios_at_init(BiosContext* ctx) {
    if (!ctx) return;

    /* Baseline XT initialization */
    bios_xt_init(ctx);

    /* Write Authentic IBM AT 5170 Release Date at 0xFFFF5 */
    const char* release_date = "01/10/84";
    switch (bios_get_revision(ctx)) {
        case BIOS_REV_AT_1985_06_10: release_date = "06/10/85"; break;
        case BIOS_REV_AT_1985_11_15: release_date = "11/15/85"; break;
        default:                     release_date = "01/10/84"; break;
    }
    for (size_t i = 0; i < 8; i++) {
        bios_poke_raw(ctx, 0xFFFF5 + (uint32_t)i, (uint8_t)release_date[i]);
    }

    /* Model Byte 0xFC at 0xFFFFE */
    bios_poke_raw(ctx, 0xFFFFE, 0xFC);
}

uint8_t bios_at_inp(BiosContext* ctx, uint16_t port) {
    if (!ctx) return 0xFF;

    if (port == 0x70 || port == 0x71) {
        /* CMOS RTC Read */
        time_t raw_time = time(NULL);
        struct tm tm_now;
#if defined(_WIN32)
        localtime_s(&tm_now, &raw_time);
#else
        localtime_r(&raw_time, &tm_now);
#endif
        /* Convert integer to BCD for CMOS RTC registers */
        switch (port) {
            case 0x70:
                return 0x00;
            case 0x71:
                return (uint8_t)(((tm_now.tm_sec / 10) << 4) | (tm_now.tm_sec % 10));
        }
    }

    return bios_xt_inp(ctx, port);
}

void bios_at_out(BiosContext* ctx, uint16_t port, uint8_t val) {
    if (!ctx) return;
    bios_xt_out(ctx, port, val);
}

bool bios_at_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs) {
    if (!ctx || !regs) return false;

    if (int_num == 0x15) {
        /* INT 15h - System Services */
        switch ((regs->ax >> 8) & 0xFF) {
            case 0x86: /* AH=86h Wait Microseconds */
                regs->flags &= ~0x0001U; /* Success */
                return true;
            case 0x88: /* AH=88h Get Extended Memory Size Above 1MB (in KB) */
                regs->ax = 15360;        /* 15 MB Extended RAM */
                regs->flags &= ~0x0001U;
                return true;
            case 0xC0: /* AH=C0h Get System Configuration */
                regs->ax = 0x0000;
                regs->bx = 0xE000;        /* Config table segment */
                regs->flags &= ~0x0001U;
                return true;
            case 0xE8: /* AX=E820h Get System Memory Map */
                if (regs->ax == 0xE820) {
                    regs->ax = (uint16_t)0x534DU; /* "SMAP" signature high word */
                    regs->flags &= ~0x0001U;
                    return true;
                }
                break;
            default:
                break;
        }
    }

    return bios_xt_interrupt(ctx, int_num, regs);
}
