// FILENAME: bios_int13.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libengine (bios.h, bios.c)
// Implements virtual BIOS interrupt and hardware emulation for bios_int13.
//
// ---- Includes ----

#include "bios/bios.h"
#include "runtime/string/memops.h"

#ifndef BIOS_DISABLE_INT13

bool bios_int13_handler(BiosContext* ctx, uint8_t int_num, BiosRegs* regs, void* user_data) {
    (void)user_data;
    if (!ctx || !regs || int_num != 0x13) return false;

    uint8_t ah = (uint8_t)(regs->ax >> 8);
    uint8_t drive = (uint8_t)(regs->dx & 0xFF);
    BiosDataArea* bda = bios_get_bda(ctx);

    switch (ah) {
        case 0x00: // Reset Disk Subsystem
            if (bda) {
                if (drive & 0x80) bda->fixed_disk_status = 0x00;
                else bda->diskette_last_err = 0x00;
            }
            regs->ax = 0x0000;
            regs->flags &= ~0x0001; // Clear Carry Flag (Success)
            return true;

        case 0x01: // Read Status of Last Operation
            if (bda) {
                uint8_t status = (drive & 0x80) ? bda->fixed_disk_status : bda->diskette_last_err;
                regs->ax = (uint16_t)(status << 8);
            }
            regs->flags &= ~0x0001;
            return true;

        case 0x08: // Read Drive Parameters
            if (drive & 0x80) {
                // Hard Disk Drive
                regs->ax = 0x0000;
                regs->bx = 0x0003; // Drive type 3
                regs->cx = 0x2710; // 615 Cylinders, 17 Sectors/Track
                regs->dx = 0x0401; // 5 Heads, 1 Drive
                regs->flags &= ~0x0001;
            } else {
                // Floppy Disk Drive
                regs->ax = 0x0000;
                regs->bx = 0x0004; // 1.44MB 3.5" Floppy
                regs->cx = 0x4F12; // 80 Tracks, 18 Sectors/Track
                regs->dx = 0x0102; // 2 Heads, 2 Drives
                regs->flags &= ~0x0001;
            }
            return true;

        default:
            break;
    }

    return false;
}

#endif // BIOS_DISABLE_INT13
