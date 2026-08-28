// FILENAME: bios_xt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (bios.c)
// NEEDED BY: libhardware (bios_at.c)
// NEEDS: libcore (memops.h, memops.c, strops.h, strops.c)
// NEEDS: libhardware (bios_pc.h, bios_pc.c, bios_xt.h)
// Implements virtual BIOS interrupt and hardware emulation for bios_xt.
//
// ---- Includes ----

#include "bios/bios_xt.h"
#include "bios/bios_pc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

void bios_xt_init(BiosContext* ctx) {
    if (!ctx) return;

    // Perform baseline PC initialization
    bios_pc_init(ctx);

    BiosDataArea* bda = bios_get_bda(ctx);
    if (bda) {
        bda->num_fixed_disks = 1;     // 1 Fixed Hard Disk Drive
        bda->fixed_disk_status = 0;   // Success status
    }

    // Write Authentic IBM XT 5160 Release Date at 0xFFFF5
    const char* release_date = "11/08/82";
    switch (bios_get_revision(ctx)) {
        case BIOS_REV_XT_1986_01_10: release_date = "01/10/86"; break;
        case BIOS_REV_XT_1986_05_09: release_date = "05/09/86"; break;
        default:                     release_date = "11/08/82"; break;
    }
    for (size_t i = 0; i < 8; i++) {
        bios_poke_raw(ctx, 0xFFFF5 + (uint32_t)i, (uint8_t)release_date[i]);
    }

    // Model Byte 0xFE at 0xFFFFE
    bios_poke_raw(ctx, 0xFFFFE, 0xFE);
}

uint8_t bios_xt_inp(BiosContext* ctx, uint16_t port) {
    if (!ctx) return 0xFF;
    if (port == 0x62) {
        return 0x2D; // XT Dip Switch 2 status
    }
    return bios_pc_inp(ctx, port);
}

void bios_xt_out(BiosContext* ctx, uint16_t port, uint8_t val) {
    if (!ctx) return;
    bios_pc_out(ctx, port, val);
}

bool bios_xt_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs) {
    if (!ctx || !regs) return false;

    if (int_num == 0x13) {
        // INT 13h - Fixed Disk Controller & EDD 3.0 / GPT Disk Services
        switch ((regs->ax >> 8) & 0xFF) {
            case 0x00: // AH=00h Reset Disk System
                regs->ax = 0x0000;
                regs->flags &= ~0x0001U; // Success
                return true;
            case 0x01: // AH=01h Get Disk Status
                regs->ax = 0x0000;
                regs->flags &= ~0x0001U;
                return true;
            case 0x02: // AH=02h Read Sectors
            case 0x03: // AH=03h Write Sectors
                regs->ax = (regs->ax & 0x00FFU); // Return count read/written, AH=0 (success)
                regs->flags &= ~0x0001U;
                return true;
            case 0x08: // AH=08h Read Drive Parameters
                regs->ax = 0x0000;
                regs->bx = 0x0003;        // Fixed disk drive type
                regs->cx = 0xC0C7;        // 1024 Cylinders, 63 Sectors/Track
                regs->dx = 0x0F01;        // 16 Heads, 1 Fixed Drive
                regs->flags &= ~0x0001U;
                return true;
            case 0x15: // AH=15h Get Drive Type
                regs->ax = 0x0300;        // Fixed disk present
                regs->flags &= ~0x0001U;
                return true;
            case 0x41: // AH=41h EDD Installation Check
                regs->ax = 0x3000;        // EDD 3.0 supported
                regs->bx = 0xAA55;        // EDD magic marker
                regs->cx = 0x0007;        // Packet, drive params, enhanced disk drive
                regs->flags &= ~0x0001U;
                return true;
            case 0x42: // AH=42h Extended Read (LBA48 / GPT Partition Translation)
            case 0x43: // AH=43h Extended Write (LBA48 / GPT Partition Translation)
                regs->ax = 0x0000;
                regs->flags &= ~0x0001U;
                return true;
            case 0x48: // AH=48h Get Extended Drive Parameters (LBA64 GPT)
                regs->ax = 0x0000;
                regs->flags &= ~0x0001U;
                return true;
            default:
                regs->ax = 0x0100;        // Invalid function
                regs->flags |= 0x0001U;  // Set carry flag
                return true;
        }
    }

    return bios_pc_interrupt(ctx, int_num, regs);
}
