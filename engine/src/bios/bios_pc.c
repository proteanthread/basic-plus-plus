// FILENAME: bios_pc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (bios.c)
// NEEDED BY: libhardware (bios_jr.c, bios_xt.c)
// NEEDS: libcore (hal.h, memops.h, memops.c, strops.h, strops.c)
// NEEDS: libengine (bgi.h, bgi.c)
// NEEDS: libhardware (bios_pc.h)
// NEEDS: libkernel (bgi_palette.h, bgi_palette.c)
// NEEDS: libplatform (platform.h)
// Implements virtual BIOS interrupt and hardware emulation for bios_pc.
//
// ---- Includes ----

#include "bios/bios_pc.h"
#include "device/bgi.h"
#include "platform/platform.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"


void bios_pc_init(BiosContext* ctx) {
    if (!ctx) return;

    BiosDataArea* bda = bios_get_bda(ctx);
    if (bda) {
        runtime_memset(bda, 0, sizeof(BiosDataArea));
        bda->com_port[0] = 0x03F8; // COM1
        bda->com_port[1] = 0x02F8; // COM2
        bda->lpt_port[0] = 0x0378; // LPT1
        bda->lpt_port[1] = 0x0278; // LPT2

        // Equipment: 80x25 Color, 2 Floppy Drives, Coprocessor Present (0x002D)
        bda->equipment_word = 0x002D;
        bda->memory_size_kb = 640;   // 640 KB Conventional RAM

        bda->kbd_start_off = 0x001E;
        bda->kbd_end_off   = 0x003E;
        bda->kbd_buf_head  = 0x001E;
        bda->kbd_buf_tail  = 0x001E;

        bda->video_mode     = 0x03;   // 80x25 16-Color Text
        bda->video_cols     = 80;
        bda->video_page_size= 4096;
        bda->crtc_base_port = 0x03D4; // Color CRTC Base
        bda->timer_ticks    = 0;
    }

    // Initialize IVT Vectors in 1MB memory array
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t ivt_addr = i * 4;
        uint16_t offset   = (uint16_t)(0xE000 + (i * 16));
        uint16_t segment  = 0xF000;
        bios_poke(ctx, ivt_addr + 0, (uint8_t)(offset & 0xFF));
        bios_poke(ctx, ivt_addr + 1, (uint8_t)((offset >> 8) & 0xFF));
        bios_poke(ctx, ivt_addr + 2, (uint8_t)(segment & 0xFF));
        bios_poke(ctx, ivt_addr + 3, (uint8_t)((segment >> 8) & 0xFF));
    }

    // Write Authentic IBM PC 5150 Copyright Notice at 0xFE000
    const char* copyright = "(C) COPYRIGHT IBM CORP 1981. ALL RIGHTS RESERVED.";
    size_t cpy_len = strlen(copyright);
    for (size_t i = 0; i < cpy_len; i++) {
        bios_poke_raw(ctx, 0xFE000 + (uint32_t)i, (uint8_t)copyright[i]);
    }

    // Write Authentic IBM PC 5150 Release Date at 0xFFFF5
    const char* release_date = "04/24/81";
    switch (bios_get_revision(ctx)) {
        case BIOS_REV_PC_1981_10_19: release_date = "10/19/81"; break;
        case BIOS_REV_PC_1982_08_16: release_date = "08/16/82"; break;
        case BIOS_REV_PC_1982_10_27: release_date = "10/27/82"; break;
        default:                     release_date = "04/24/81"; break;
    }
    for (size_t i = 0; i < 8; i++) {
        bios_poke_raw(ctx, 0xFFFF5 + (uint32_t)i, (uint8_t)release_date[i]);
    }

    // Model Byte 0xFF at 0xFFFFE
    bios_poke_raw(ctx, 0xFFFFE, 0xFF);

    // Populate IBM PC 8x8 ROM Character Font Generator at 0xF000:0xFA6E (0xFFA6E)
    const uint8_t *font_rom = BGI_get_font_8x8();
    if (font_rom) {
        for (uint32_t i = 0; i < 2048; i++) {
            bios_poke_raw(ctx, 0xFFA6E + i, font_rom[i]);
        }
    }

    // Set INT 1Fh (0x0007C) and INT 43h (0x0010C) font table pointers to 0xF000:0xFA6E
    bios_poke(ctx, 0x0007C, 0x6E);
    bios_poke(ctx, 0x0007D, 0xFA);
    bios_poke(ctx, 0x0007E, 0x00);
    bios_poke(ctx, 0x0007F, 0xF0);

    bios_poke(ctx, 0x0010C, 0x6E);
    bios_poke(ctx, 0x0010D, 0xFA);
    bios_poke(ctx, 0x0010E, 0x00);
    bios_poke(ctx, 0x0010F, 0xF0);
}

uint8_t bios_pc_inp(BiosContext* ctx, uint16_t port) {
    if (!ctx) return 0xFF;
    switch (port) {
        case 0x40: // PIT Counter 0 (timer ticks)
        case 0x41:
        case 0x42: {
            BiosDataArea* bda = bios_get_bda(ctx);
            return (uint8_t)(bda ? (bda->timer_ticks & 0xFF) : 0x00);
        }
        case 0x60: // 8255 PPI Port A - Keyboard Scan Code
            return 0x00; // No key pending
        case 0x61: // 8255 PPI Port B - System Control Bits
            return 0x20;
        case 0x62: // 8255 PPI Port C - Dip Switches
            return 0x2D; // 640KB RAM, CGA, 2 Floppy Drives
        case 0x3D4:
        case 0x3D5: // 6845 CRTC Controller Registers
            return 0x00;
        default:
            return 0xFF;
    }
}

#include "device/bgi_palette.h"

void bios_pc_out(BiosContext* ctx, uint16_t port, uint8_t val) {
    if (!ctx) return;
    if (port == 0x03C8 || port == 0x03C9) {
        bgi_palette_handle_port_write(bgi_palette_get_global(), port, val);
    }
}

bool bios_pc_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs) {
    if (!ctx || !regs) return false;
    BiosDataArea* bda = bios_get_bda(ctx);

    switch (int_num) {
        case 0x10: // INT 10h - Video Services
            switch ((regs->ax >> 8) & 0xFF) {
                case 0x00: // AH=00h Set Video Mode
                    if (bda) bda->video_mode = (uint8_t)(regs->ax & 0x7F);
                    regs->flags &= ~0x0001U; // Clear carry flag
                    return true;
                case 0x02: // AH=02h Set Cursor Position
                    if (bda) {
                        uint8_t page = (uint8_t)((regs->bx >> 8) & 0x07);
                        bda->cursor_pos[page] = (regs->dx & 0xFFFF);
                    }
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x03: // AH=03h Get Cursor Position
                    if (bda) {
                        uint8_t page = (uint8_t)((regs->bx >> 8) & 0x07);
                        regs->dx = bda->cursor_pos[page];
                        regs->cx = bda->cursor_lines;
                    }
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x0E: // AH=0Eh Teletype Output
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x0F: // AH=0Fh Get Video State
                    if (bda) {
                        regs->ax = (uint16_t)((bda->video_mode << 8) | bda->video_cols);
                        regs->bx = (uint16_t)(bda->active_page << 8);
                    }
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x4F: // AH=4Fh VESA VBE 2.0/3.0 Services
                    switch (regs->ax & 0xFFFF) {
                        case 0x4F00: // Return VBE Controller Information
                            regs->ax = 0x004F; // VBE Success (AL=4Fh, AH=00h)
                            regs->flags &= ~0x0001U;
                            return true;
                        case 0x4F01: // Return VBE Mode Information
                            regs->ax = 0x004F;
                            regs->cx = 0x0101; // Mode 640x480 256-color
                            regs->flags &= ~0x0001U;
                            return true;
                        case 0x4F02: // Set VBE Mode
                        case 0x4F03: // Get VBE Mode
                            regs->ax = 0x004F;
                            regs->flags &= ~0x0001U;
                            return true;
                        default:
                            regs->ax = 0x014F; // VBE Function Not Supported
                            return true;
                    }
                default:
                    regs->flags &= ~0x0001U;
                    return true;
            }

        case 0x11: // INT 11h - Get Equipment List
            regs->ax = bda ? bda->equipment_word : 0x002D;
            regs->flags &= ~0x0001U;
            return true;

        case 0x12: // INT 12h - Get Memory Size
            regs->ax = bda ? bda->memory_size_kb : 640;
            regs->flags &= ~0x0001U;
            return true;

        case 0x14: // INT 14h - Serial Communications Services
            switch ((regs->ax >> 8) & 0xFF) {
                case 0x00: // AH=00h Initialize Serial Port
                case 0x01: // AH=01h Transmit Character
                case 0x02: // AH=02h Receive Character
                case 0x03: // AH=03h Get Serial Port Status
                    regs->ax = 0x6000; // Line ready, transmitter empty
                    regs->flags &= ~0x0001U;
                    return true;
                default:
                    regs->flags &= ~0x0001U;
                    return true;
            }

        case 0x15: // INT 15h - System & Joystick Services
            switch ((regs->ax >> 8) & 0xFF) {
                case 0x84: // AH=84h Joystick / Game Port API
                    if ((regs->dx & 0xFF) == 0x00) {
                        // Read Joystick Switches/Buttons: AL=0xF0 (no buttons pressed)
                        regs->ax = 0x00F0;
                        regs->flags &= ~0x0001U;
                    } else if ((regs->dx & 0xFF) == 0x01) {
                        // Read Joystick Potentiometer Coordinates
                        regs->ax = 128; // X1
                        regs->bx = 128; // Y1
                        regs->cx = 128; // X2
                        regs->dx = 128; // Y2
                        regs->flags &= ~0x0001U;
                    }
                    return true;
                case 0xC0: // AH=C0h Get Configuration Parameters
                    regs->ax = 0x0000;
                    regs->bx = 0x0000;
                    regs->flags &= ~0x0001U;
                    return true;
                default:
                    regs->flags &= ~0x0001U;
                    return true;
            }

        case 0x16: // INT 16h - Keyboard Services
            switch ((regs->ax >> 8) & 0xFF) {
                case 0x00: // AH=00h Read Keystroke
                    regs->ax = 0x0000;
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x01: // AH=01h Check Keystroke Status
                    regs->flags |= 0x0040U; // Zero Flag set = no key available
                    return true;
                case 0x02: // AH=02h Get Shift Flags
                    regs->ax = (uint16_t)(bda ? bda->kbd_flag_1 : 0x00);
                    regs->flags &= ~0x0001U;
                    return true;
                default:
                    regs->flags &= ~0x0001U;
                    return true;
            }

        case 0x17: // INT 17h - Parallel Printer Services
            switch ((regs->ax >> 8) & 0xFF) {
                case 0x00: // AH=00h Print Character
                case 0x01: // AH=01h Initialize Printer
                case 0x02: // AH=02h Get Printer Status
                    regs->ax = 0x9000; // Selected & not busy
                    regs->flags &= ~0x0001U;
                    return true;
                default:
                    regs->flags &= ~0x0001U;
                    return true;
            }

        case 0x1A: // INT 1Ah - Real-Time Clock / Timer Services
            switch ((regs->ax >> 8) & 0xFF) {
                case 0x00: // AH=00h Read System Timer Ticks
                    if (bda) {
                        regs->cx = (uint16_t)((bda->timer_ticks >> 16) & 0xFFFF);
                        regs->dx = (uint16_t)(bda->timer_ticks & 0xFFFF);
                        regs->ax = (uint16_t)bda->timer_overflow;
                        bda->timer_overflow = 0;
                    }
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x01: // AH=01h Set System Timer Ticks
                    if (bda) {
                        bda->timer_ticks = (((uint32_t)regs->cx) << 16) | (regs->dx & 0xFFFF);
                    }
                    regs->flags &= ~0x0001U;
                    return true;
                case 0xB6: // AX=B600h PnP BIOS Installation Check
                    regs->ax = 0x0000; // $PnP present
                    regs->flags &= ~0x0001U;
                    return true;
                default:
                    regs->flags &= ~0x0001U;
                    return true;
            }

        case 0x21: // INT 21h - MS-DOS Kernel Services API
            switch ((regs->ax >> 8) & 0xFF) {
                case 0x00: // AH=00h Terminate Program
                case 0x4C: // AH=4Ch Terminate Process
                    regs->ax = 0x0000;
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x01: // AH=01h Character Input with Echo
                    regs->ax = 0x000D; // Return CR
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x02: // AH=02h Character Output
                case 0x06: // AH=06h Direct Console I/O
                case 0x09: // AH=09h Display String
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x2A: { // AH=2Ah Get System Date
                    HalContext *hal = hal_get();
                    time_t t = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
                    struct tm tm_now;
                    runtime_memset(&tm_now, 0, sizeof(tm_now));
                    platform_localtime(&t, &tm_now);
                    regs->cx = (uint16_t)(tm_now.tm_year + 1900);
                    regs->dx = (uint16_t)(((tm_now.tm_mon + 1) << 8) | tm_now.tm_mday);
                    regs->ax = (uint16_t)tm_now.tm_wday;
                    regs->flags &= ~0x0001U;
                    return true;
                }
                case 0x2C: { // AH=2Ch Get System Time
                    HalContext *hal = hal_get();
                    time_t t = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
                    struct tm tm_now;
                    runtime_memset(&tm_now, 0, sizeof(tm_now));
                    platform_localtime(&t, &tm_now);
                    regs->cx = (uint16_t)((tm_now.tm_hour << 8) | tm_now.tm_min);
                    regs->dx = (uint16_t)((tm_now.tm_sec << 8) | 0);
                    regs->flags &= ~0x0001U;
                    return true;
                }

                case 0x30: // AH=30h Get MS-DOS Version
                    regs->ax = 0x0005; // MS-DOS 5.00
                    regs->bx = 0xFF00;
                    regs->cx = 0x0000;
                    regs->flags &= ~0x0001U;
                    return true;
                default:
                    regs->flags &= ~0x0001U;
                    return true;
            }

        case 0x2F: // INT 2Fh - Multiplex & XMS Services
            if ((regs->ax & 0xFFFF) == 0x4300) {
                regs->ax = 0x0080; // XMS 3.0 Installed
                regs->flags &= ~0x0001U;
                return true;
            } else if ((regs->ax & 0xFFFF) == 0x4310) {
                regs->bx = 0x0010; // Entry offset
                regs->es = 0xF000; // Entry segment
                regs->flags &= ~0x0001U;
                return true;
            }
            return false;

        case 0x33: // INT 33h - Microsoft Mouse Driver API
            switch (regs->ax & 0xFFFF) {
                case 0x0000: // Reset Mouse / Status
                    regs->ax = 0xFFFF; // Mouse installed
                    regs->bx = 0x0003; // 3 Buttons
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x0001: // Show Cursor
                case 0x0002: // Hide Cursor
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x0003: // Get Mouse Position & Buttons
                    regs->bx = 0x0000; // No buttons pressed
                    regs->cx = 320;    // X position
                    regs->dx = 200;    // Y position
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x0004: // Set Mouse Position
                    regs->flags &= ~0x0001U;
                    return true;
                default:
                    regs->flags &= ~0x0001U;
                    return true;
            }

        case 0x67: // INT 67h - LIM EMS 4.0 Memory Manager
            switch ((regs->ax >> 8) & 0xFF) {
                case 0x40: // Get EMS Status
                    regs->ax = 0x0000; // EMS operational
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x41: // Get Page Frame Segment
                    regs->ax = 0x0000;
                    regs->bx = 0xE000; // Page Frame 0xE000
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x42: // Get Unallocated Page Count
                    regs->ax = 0x0000;
                    regs->bx = 64; // 64 pages available
                    regs->dx = 64; // 64 total pages
                    regs->flags &= ~0x0001U;
                    return true;
                case 0x46: // Get EMS Version
                    regs->ax = 0x0040; // Version 4.0
                    regs->flags &= ~0x0001U;
                    return true;
                default:
                    regs->ax = 0x0000;
                    regs->flags &= ~0x0001U;
                    return true;
            }

        default:
            return false;
    }
}
