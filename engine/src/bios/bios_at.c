// FILENAME: bios_at.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (bios.c)
// NEEDS: libcore (hal.h, memops.h, memops.c, strops.h, strops.c)
// NEEDS: libhardware (bios_at.h, bios_xt.h, bios_xt.c)
// NEEDS: libplatform (platform.h)
// Implements virtual BIOS interrupt and hardware emulation for bios_at.
//
// ---- Includes ----

#include "bios/bios_at.h"
#include "bios/bios_xt.h"
#include "platform/platform.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"

void bios_at_init(BiosContext* ctx) {
    if (!ctx) return;

    // Baseline XT initialization
    bios_xt_init(ctx);

    // Write Authentic IBM AT 5170 Release Date at 0xFFFF5
    const char* release_date = "01/10/84";
    switch (bios_get_revision(ctx)) {
        case BIOS_REV_AT_1985_06_10: release_date = "06/10/85"; break;
        case BIOS_REV_AT_1985_11_15: release_date = "11/15/85"; break;
        default:                     release_date = "01/10/84"; break;
    }
    for (size_t i = 0; i < 8; i++) {
        bios_poke_raw(ctx, 0xFFFF5 + (uint32_t)i, (uint8_t)release_date[i]);
    }

    // Model Byte 0xFC at 0xFFFFE
    bios_poke_raw(ctx, 0xFFFFE, 0xFC);
}

uint8_t bios_at_inp(BiosContext* ctx, uint16_t port) {
    if (!ctx) return 0xFF;

    if (port == 0x70 || port == 0x71) {
        // CMOS RTC Read
        HalContext *hal = hal_get();
        time_t raw_time = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
        struct tm tm_now;
        runtime_memset(&tm_now, 0, sizeof(tm_now));
        platform_localtime(&raw_time, &tm_now);
        // Convert integer to BCD for CMOS RTC registers
        if (port == 0x70) {
            return 0x00;
        } else {
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
        // INT 15h - System Services
        switch ((regs->ax >> 8) & 0xFF) {
            case 0x86: // AH=86h Wait Microseconds
                regs->flags &= ~0x0001U; // Success
                return true;
            case 0x88: // AH=88h Get Extended Memory Size Above 1MB (in KB)
                regs->ax = 15360;        // 15 MB Extended RAM
                regs->flags &= ~0x0001U;
                return true;
            case 0xC0: // AH=C0h Get System Configuration
                regs->ax = 0x0000;
                regs->bx = 0xE000;        // Config table segment
                regs->flags &= ~0x0001U;
                return true;
            case 0xE8: // AX=E820h Get System Memory Map
                if (regs->ax == 0xE820) {
                    regs->ax = (uint16_t)0x534DU; // "SMAP" signature high word
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
