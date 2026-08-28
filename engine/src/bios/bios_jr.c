// FILENAME: bios_jr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (bios.c)
// NEEDS: libcore (memops.h, memops.c, strops.h, strops.c)
// NEEDS: libhardware (bios_jr.h, bios_pc.h, bios_pc.c)
// Implements virtual BIOS interrupt and hardware emulation for bios_jr.
//
// ---- Includes ----

#include "bios/bios_jr.h"
#include "bios/bios_pc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

void bios_jr_init(BiosContext* ctx) {
    if (!ctx) return;

    // Baseline PC initialization
    bios_pc_init(ctx);

    BiosDataArea* bda = bios_get_bda(ctx);
    if (bda) {
        bda->pcjr_video_flags = 0x01; // PCjr 16-color Video Array active
    }

    // Write Authentic IBM PCjr 4860 Release Date "06/01/83" at 0xFFFF5
    const char* release_date = "06/01/83";
    for (size_t i = 0; i < 8; i++) {
        bios_poke_raw(ctx, 0xFFFF5 + (uint32_t)i, (uint8_t)release_date[i]);
    }

    // Model Byte 0xFD at 0xFFFFE
    bios_poke_raw(ctx, 0xFFFFE, 0xFD);
}

uint8_t bios_jr_inp(BiosContext* ctx, uint16_t port) {
    if (!ctx) return 0xFF;
    if (port == 0x03DA || port == 0x03DF) {
        return 0x00; // PCjr Video Array Status
    }
    if (port == 0xC0) {
        return 0x00; // SN76496 PSG Sound Generator Status
    }
    return bios_pc_inp(ctx, port);
}

void bios_jr_out(BiosContext* ctx, uint16_t port, uint8_t val) {
    if (!ctx) return;
    if (port == 0xC0) {
        // SN76496 PSG Sound Data Register Write
        return;
    }
    bios_pc_out(ctx, port, val);
}

bool bios_jr_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs) {
    if (!ctx || !regs) return false;
    return bios_pc_interrupt(ctx, int_num, regs);
}
