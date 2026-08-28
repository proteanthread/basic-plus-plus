// FILENAME: bios_int16.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libengine (bios.h, bios.c)
// Implements virtual BIOS interrupt and hardware emulation for bios_int16.
//
// ---- Includes ----

#include "bios/bios.h"
#include "runtime/string/memops.h"

#ifndef BIOS_DISABLE_INT16

bool bios_int16_handler(BiosContext* ctx, uint8_t int_num, BiosRegs* regs, void* user_data) {
    (void)user_data;
    if (!ctx || !regs || int_num != 0x16) return false;

    uint8_t ah = (uint8_t)(regs->ax >> 8);
    BiosDataArea* bda = bios_get_bda(ctx);

    switch (ah) {
        case 0x00: // Read Key Character
            if (bda) {
                if (bda->kbd_buf_head != bda->kbd_buf_tail) {
                    uint16_t idx = (bda->kbd_buf_head - bda->kbd_start_off) / 2;
                    if (idx < 16) {
                        regs->ax = bda->kbd_buffer[idx];
                        bda->kbd_buf_head += 2;
                        if (bda->kbd_buf_head >= bda->kbd_end_off) {
                            bda->kbd_buf_head = bda->kbd_start_off;
                        }
                    }
                } else {
                    regs->ax = 0x0000; // Buffer empty
                }
            }
            return true;

        case 0x01: // Check Keyboard Buffer Status
            if (bda) {
                if (bda->kbd_buf_head != bda->kbd_buf_tail) {
                    uint16_t idx = (bda->kbd_buf_head - bda->kbd_start_off) / 2;
                    if (idx < 16) {
                        regs->ax = bda->kbd_buffer[idx];
                    }
                    regs->flags &= ~0x0040; // Clear Zero Flag (Key Ready)
                } else {
                    regs->flags |= 0x0040;  // Set Zero Flag (Buffer Empty)
                }
            }
            return true;

        case 0x02: // Read Shift Flags
            if (bda) {
                regs->ax = (uint16_t)(bda->kbd_flag_1 & 0xFF);
            }
            return true;

        default:
            break;
    }

    return false;
}

#endif // BIOS_DISABLE_INT16
