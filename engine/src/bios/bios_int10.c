// FILENAME: bios_int10.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libengine (bios.h, bios.c)
// Implements virtual BIOS interrupt and hardware emulation for bios_int10.
//
// ---- Includes ----

#include "bios/bios.h"
#include "runtime/string/memops.h"

#ifndef BIOS_DISABLE_INT10

bool bios_int10_handler(BiosContext* ctx, uint8_t int_num, BiosRegs* regs, void* user_data) {
    (void)user_data;
    if (!ctx || !regs || int_num != 0x10) return false;

    uint8_t ah = (uint8_t)(regs->ax >> 8);
    uint8_t al = (uint8_t)(regs->ax & 0xFF);

    BiosDataArea* bda = bios_get_bda(ctx);

    switch (ah) {
        case 0x00: // Set Video Mode
            if (bda) {
                bda->video_mode = al;
                bda->video_cols = (al == 0x00 || al == 0x01) ? 40 : 80;
                bda->active_page = 0;
            }
            return true;

        case 0x02: // Set Cursor Position
            if (bda) {
                uint8_t page = (uint8_t)(regs->bx >> 8);
                if (page < 8) {
                    bda->cursor_pos[page] = regs->dx;
                }
            }
            return true;

        case 0x03: // Read Cursor Position
            if (bda) {
                uint8_t page = (uint8_t)(regs->bx >> 8);
                if (page < 8) {
                    regs->dx = bda->cursor_pos[page];
                }
                regs->cx = bda->cursor_lines;
            }
            return true;

        case 0x0E: // Teletype Output
            // Writes character in AL to current cursor position and advances cursor
            if (bda) {
                uint8_t page = bda->active_page;
                uint8_t row = (uint8_t)(bda->cursor_pos[page] >> 8);
                uint8_t col = (uint8_t)(bda->cursor_pos[page] & 0xFF);

                if (al == '\r') {
                    col = 0;
                } else if (al == '\n') {
                    row++;
                } else {
                    col++;
                    if (col >= bda->video_cols) {
                        col = 0;
                        row++;
                    }
                }
                bda->cursor_pos[page] = (uint16_t)((row << 8) | col);
            }
            return true;

        default:
            break;
    }

    return false;
}

#endif // BIOS_DISABLE_INT10
