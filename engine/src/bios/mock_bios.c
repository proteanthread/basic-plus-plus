/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/* Enable POSIX extensions (localtime_r) on Linux/POSIX systems.
 * Must be defined before any system header includes. */
#if !defined(_MSC_VER) && !defined(__WATCOMC__) && !defined(MSDOS)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * @file mock_bios.c
 * @brief Switchable IBM PC BIOS & DOS Interrupt Emulator implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements memory mapping (IVT, BDA), standard PC port emulation,
 *   and x86 interrupt vectors (INT 10h, 16h, 21h, etc.) in a highly portable format.
 * - Why it exists: Simulates BIOS and DOS runtime layers for retro emulation and
 *   allows vintage hardware routines to operate inside sandboxed interpreters.
 * - Why it works this way: It interfaces with the CPU register state and memory space
 *   solely via user-supplied callbacks.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Port behaviors, adding CMOS register defaults, interrupt subfunctions.
 * - What cannot be changed: BDA offset memory locations (must follow IBM PC memory layout).
 * - What to expect: Zero dynamic memory allocation. Complete safety.
 * - What to do if something breaks: Check address bounds in read_mem/write_mem callbacks.
 */

#define BPP_BIOS_DLL_EXPORT
#include "bios/mock_bios.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "platform/platform.h"

#include "types/config.h"

/* BDA offset helper macros */
#define BDA_COM1_PORT   0x0400
#define BDA_COM2_PORT   0x0402
#define BDA_LPT1_PORT   0x0408
#define BDA_LPT2_PORT   0x040A
#define BDA_EQUIP_WORD  0x0410
#define BDA_MEM_SIZE    0x0413
#define BDA_KBD_HEAD    0x041A
#define BDA_KBD_TAIL    0x041C
#define BDA_VIDEO_MODE  0x0449
#define BDA_COLUMNS     0x044A
#define BDA_CRT_BASE    0x0463
#define BDA_TIMER_TICKS 0x046C

static void write16_direct(uint8_t *mem, size_t size, uint32_t addr, uint16_t val) {
    if (addr + 1 < size) {
        mem[addr] = (uint8_t)(val & 0xFF);
        mem[addr + 1] = (uint8_t)((val >> 8) & 0xFF);
    }
}

static uint16_t read_bda16(MockBiosContext *ctx, uint32_t addr) {
    uint8_t low = ctx->read_mem(ctx->user_data, addr);
    uint8_t high = ctx->read_mem(ctx->user_data, addr + 1);
    return (uint16_t)((high << 8) | low);
}

static void write_bda16(MockBiosContext *ctx, uint32_t addr, uint16_t val) {
    ctx->write_mem(ctx->user_data, addr, (uint8_t)(val & 0xFF));
    ctx->write_mem(ctx->user_data, addr + 1, (uint8_t)((val >> 8) & 0xFF));
}

void mock_bios_init_mem(MockBiosContext *ctx, uint8_t *mem, size_t size, MockBiosModel model) {
    ctx->model = model;
    ctx->key_buffer = 0;
    ctx->cmos_idx = 0;
    memset(ctx->cmos_regs, 0, sizeof(ctx->cmos_regs));
    ctx->ppi_port_b = 0;
    ctx->pit_counter = 0;
    ctx->crtc_index = 0;
    memset(ctx->crtc_regs, 0, sizeof(ctx->crtc_regs));

    for (int i = 0; i < 20; i++) {
        ctx->dos_handles[i] = NULL;
    }

    if (model == BIOS_MODEL_NONE) {
        if (size > 0) memset(mem, 0, size);
        return;
    }

    /* Zero out memory initially for all models */
    if (size > 0) memset(mem, 0, size);

    /* 1. Initialize non-PC legacy machines */
    if (model == BIOS_MODEL_C64) {
        /* C64 Zero Page BASIC pointers */
        write16_direct(mem, size, 0x002B, 0x0801); /* TXTTAB */
        write16_direct(mem, size, 0x002D, 0x0803); /* VARTAB */
        write16_direct(mem, size, 0x002F, 0x0803); /* ARYTAB */
        write16_direct(mem, size, 0x0031, 0x0803); /* STREND */
        write16_direct(mem, size, 0x0033, 0xA000); /* FRETOP */
        write16_direct(mem, size, 0x0037, 0xA000); /* MEMSIZ */
        if (0x00BA < size) mem[0x00BA] = 0x08;      /* Default drive 8 */
        if (0x0286 < size) mem[0x0286] = 0x0E;      /* Light blue text color */
        if (0x0001 < size) {
            mem[0x0000] = 0x2F;
            mem[0x0001] = 0x37;
        }
        /* VIC-II Screen background/border color registers */
        if (53281 < size) {
            mem[53280] = 14; /* Border: Light Blue */
            mem[53281] = 6;  /* Background: Blue */
        }
        return;
    }
    else if (model == BIOS_MODEL_VIC20) {
        /* VIC-20 Zero Page BASIC pointers */
        write16_direct(mem, size, 0x002B, 0x1001); /* Start of BASIC text */
        write16_direct(mem, size, 0x002D, 0x1003); /* Start of variables */
        write16_direct(mem, size, 0x0037, 0x1E00); /* Top of BASIC memory */
        /* VIC-I chip register defaults */
        if (0x900F < size) {
            mem[0x9000] = 0x0C;
            mem[0x9001] = 0x26;
            mem[0x9002] = 0x96;
            mem[0x9003] = 0x2E;
            mem[0x9005] = 0xF0;
            mem[0x900F] = 0x1B;
        }
        /* Pre-fill screen memory with space character (0x20) */
        if (0x1E00 + 506 <= size) {
            memset(mem + 0x1E00, 0x20, 506);
        }
        /* Pre-fill color memory with white (0x01) */
        if (0x9600 + 506 <= size) {
            memset(mem + 0x9600, 0x01, 506);
        }
        return;
    }
    else if (model == BIOS_MODEL_ATARI) {
        /* Atari 8-Bit Zero Page OS Variables */
        if (0x0059 < size) {
            mem[0x52] = 2;                             /* LMARGN */
            mem[0x53] = 39;                            /* RMARGN */
            write16_direct(mem, size, 0x0058, 0x9C40); /* SAVMSC (screen memory base) */
        }
        write16_direct(mem, size, 0x0080, 0xA000); /* LOMEM */
        write16_direct(mem, size, 0x0090, 0xBC1F); /* MEMTOP */
        /* Color shadow registers */
        if (0x02C8 < size) {
            mem[0x2C4] = 0x28; /* COLPF0 */
            mem[0x2C5] = 0xCA; /* COLPF1 */
            mem[0x2C6] = 0x94; /* COLPF2 */
            mem[0x2C7] = 0x46; /* COLPF3 */
            mem[0x2C8] = 0x00; /* COLBK (background/border) */
        }
        /* ANTIC defaults */
        if (0xD403 < size) {
            mem[0xD400] = 0x22;                        /* DMACTL */
            mem[0xD401] = 0x02;                        /* CHACTL */
            write16_direct(mem, size, 0xD402, 0x9C20); /* DLISTL/H (display list pointer) */
        }
        return;
    }
    else if (model == BIOS_MODEL_APPLE2) {
        /* Apple IIe Applesoft Zero Page */
        write16_direct(mem, size, 0x0067, 0x0801); /* TXTTAB */
        write16_direct(mem, size, 0x0069, 0x0803); /* VARTAB */
        write16_direct(mem, size, 0x006B, 0x0803); /* ARYTAB */
        write16_direct(mem, size, 0x006D, 0x0803); /* STREND */
        write16_direct(mem, size, 0x0073, 0x9600); /* HIMEM */
        write16_direct(mem, size, 0x00AF, 0x9600); /* MEMSIZ */
        /* Text Page 1: 1024 bytes filled with 0xA0 (spaces with high bit set) */
        if (0x0400 + 1024 <= size) {
            memset(mem + 0x0400, 0xA0, 1024);
        }
        return;
    }
    else if (model == BIOS_MODEL_TRS80) {
        /* TRS-80 Boot ROM identifiers */
        if (0x0062 < size) {
            mem[0x0000] = 0xF3;
            mem[0x0001] = 0xAF;
            mem[0x0062] = 0x52; /* 'R' identification */
        }
        /* Video RAM: 1024 bytes filled with space character (0x20) */
        if (0x3C00 + 1024 <= size) {
            memset(mem + 0x3C00, 0x20, 1024);
        }
        /* BASIC System Variables */
        write16_direct(mem, size, 0x4000, 0x4200); /* Start of BASIC text */
        write16_direct(mem, size, 0x4002, 0x7FFF); /* Top of RAM */
        return;
    }

    /* 2. Initialize PC-compatible models (MSDOS, IBMPC, PCJR, PCXT, PCAT, TANDY) */
    /* Initialize IVT (Interrupt Vector Table) pointing to ROM segment 0xF000 */
    for (int i = 0; i < 256; i++) {
        uint32_t ivt_addr = i * 4;
        uint16_t ip_offset = (uint16_t)(0x1000 + (i * 16));
        uint16_t cs_segment = 0xF000;
        write16_direct(mem, size, ivt_addr, ip_offset);
        write16_direct(mem, size, ivt_addr + 2, cs_segment);
    }

    /* Initialize BDA (BIOS Data Area) base settings */
    if (0x0400 + 256 <= size) {
        write16_direct(mem, size, BDA_COM1_PORT, 0x03F8); /* COM1 base port */
        write16_direct(mem, size, BDA_COM2_PORT, 0x02F8); /* COM2 base port */
        write16_direct(mem, size, BDA_LPT1_PORT, 0x0378); /* LPT1 base port */
        write16_direct(mem, size, BDA_LPT2_PORT, 0x0278); /* LPT2 base port */
        write16_direct(mem, size, BDA_MEM_SIZE, 0x0280);  /* 640 KB conventional RAM */

        mem[BDA_VIDEO_MODE] = 0x03;                       /* Default Video Mode 3 (Color 80x25) */
        write16_direct(mem, size, BDA_COLUMNS, 80);       /* 80 columns */
        write16_direct(mem, size, BDA_CRT_BASE, 0x03D4);  /* CRT base address (CGA) */

        write16_direct(mem, size, BDA_KBD_HEAD, 0x001E);  /* Keyboard buffer head */
        write16_direct(mem, size, BDA_KBD_TAIL, 0x001E);  /* Keyboard buffer tail */

        /* Equipment Word Flags */
        uint16_t equip = 0x022D; /* 1 floppy, 80x25 color, 2 serial, 1 printer */
        write16_direct(mem, size, BDA_EQUIP_WORD, equip);

        /* Model-specific overrides */
        if (model == BIOS_MODEL_PCJR || model == BIOS_MODEL_TANDY) {
            mem[BDA_VIDEO_MODE] = 0x01;                 /* Mode 1 (PCjr 40x25) */
            write16_direct(mem, size, BDA_COLUMNS, 40); /* 40 columns */
            if (model == BIOS_MODEL_TANDY && 0x03DA < size) {
                mem[0x03DA] = 0x00;                     /* Tandy TGA register default */
            }
        } else if (model == BIOS_MODEL_XT) {
            mem[0x0475] = 1; /* 1 Hard Disk drive */
        } else if (model == BIOS_MODEL_AT) {
            mem[0x0496] = 0x10; /* AT keyboard layout flags */
        }
    }

    /* 3. Pre-populate extended BIOS ROM cartridge & extensions */
    if (model == BIOS_MODEL_PCJR) {
        /* PCjr Cartridge signature in segment D000h */
        if (0xD0004 < size) {
            mem[0xD0000] = 0x55;
            mem[0xD0001] = 0xAA;
            mem[0xD0002] = 0x08; /* 8 blocks size (4 KB) */
            mem[0xD0003] = 0x00; /* Entry offset */
            mem[0xD0004] = 0x01; /* Entry segment offset */
        }
    } else if (model == BIOS_MODEL_XT || model == BIOS_MODEL_AT) {
        /* XT Hard Disk / IDE Extension signature in segment C800h */
        if (0xC8002 < size) {
            mem[0xC8000] = 0x55;
            mem[0xC8001] = 0xAA;
            mem[0xC8002] = 0x10; /* 8 KB size */
        }
    }

    /* 4. Set BIOS model byte at F000:FFFE -> 0xFFFFE */
    if (0xFFFFE < size) {
        uint8_t model_byte = 0xFF; /* PC (Default) */
        if (model == BIOS_MODEL_PCJR) model_byte = 0xFD;
        else if (model == BIOS_MODEL_XT) model_byte = 0xFE;
        else if (model == BIOS_MODEL_AT) model_byte = 0xFC;
        mem[0xFFFFE] = model_byte;
    }
}

uint8_t mock_bios_in(MockBiosContext *ctx, uint16_t port) {
    if (ctx->model == BIOS_MODEL_NONE) return 0xFF;

    /* PIT Counter emulations (40h-43h) */
    if (port >= 0x40 && port <= 0x43) {
        ctx->pit_counter++;
        return ctx->pit_counter;
    }

    /* PPI System Control Port B (61h) */
    if (port == 0x61) {
        return ctx->ppi_port_b;
    }

    /* CGA CRT registers (3D4h-3D5h) */
    if (port == 0x3D5) {
        if (ctx->crtc_index < 32) {
            return ctx->crtc_regs[ctx->crtc_index];
        }
        return 0;
    }

    /* PIC controller (20h-21h) */
    if (port == 0x21) {
        return 0x00; /* All interrupts enabled */
    }

    /* CMOS RAM data register (71h) */
    if (port == 0x71 && ctx->model == BIOS_MODEL_AT) {
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt;
        lt = platform_localtime(&t, &tm_buf);
        switch (ctx->cmos_idx) {
            case 0x00: return lt ? (uint8_t)lt->tm_sec : 0;
            case 0x02: return lt ? (uint8_t)lt->tm_min : 0;
            case 0x04: return lt ? (uint8_t)lt->tm_hour : 0;
            case 0x06: return lt ? (uint8_t)lt->tm_wday : 0;
            case 0x07: return lt ? (uint8_t)lt->tm_mday : 0;
            case 0x08: return lt ? (uint8_t)(lt->tm_mon + 1) : 0;
            case 0x09: return lt ? (uint8_t)(lt->tm_year % 100) : 0;
            case 0x10: return 0x44; /* Standard Floppy layout */
            case 0x12: return 0x00; /* Standard Hard Disk drive */
            case 0x15: return 0x80; /* Base memory low byte (640K) */
            case 0x16: return 0x02; /* Base memory high byte (640K) */
            case 0x17: return 0x00; /* Extended memory low byte (1024K) */
            case 0x18: return 0x04; /* Extended memory high byte (1024K) */
            default:   return ctx->cmos_regs[ctx->cmos_idx & 0x3F];
        }
    }

    return 0xFF;
}

void mock_bios_out(MockBiosContext *ctx, uint16_t port, uint8_t val) {
    if (ctx->model == BIOS_MODEL_NONE) return;

    /* PPI System Control Port B (61h) */
    if (port == 0x61) {
        ctx->ppi_port_b = val;
    }

    /* CGA CRT registers (3D4h-3D5h) */
    if (port == 0x3D4) {
        ctx->crtc_index = val & 0x1F;
    } else if (port == 0x3D5) {
        if (ctx->crtc_index < 32) {
            ctx->crtc_regs[ctx->crtc_index] = val;
        }
    }

    /* CMOS RAM index register (70h) */
    if (port == 0x70 && ctx->model == BIOS_MODEL_AT) {
        ctx->cmos_idx = val & 0x3F;
    } else if (port == 0x71 && ctx->model == BIOS_MODEL_AT) {
        ctx->cmos_regs[ctx->cmos_idx & 0x3F] = val;
    }
}

void mock_bios_interrupt(MockBiosContext *ctx, uint8_t int_num) {
    uint32_t ax = 0, bx = 0, cx = 0, dx = 0, flags = 0;
    if (ctx->get_registers) {
        ctx->get_registers(ctx->user_data, &ax, &bx, &cx, &dx, &flags);
    }
    flags &= ~0x01; /* Clear Carry Flag */

    int is_at = (ctx->model == BIOS_MODEL_AT);
    MockBiosRegs regs = { ax, bx, cx, dx, flags };
    int handled = 0;

    /* --- INT 10h: Video Services --- */
    if (int_num == 0x10) {
        uint8_t ah = (uint8_t)((ax >> 8) & 0xFF);
        uint8_t al = (uint8_t)(ax & 0xFF);

        if (ah == 0x00) {
            /* Set Video Mode */
            if (ctx->write_mem) {
                ctx->write_mem(ctx->user_data, BDA_VIDEO_MODE, al);
                ctx->write_mem(ctx->user_data, BDA_COLUMNS, (al == 0 || al == 1) ? 40 : 80);
                ctx->write_mem(ctx->user_data, 0x044B, 0); /* Video page 0 offset */
                if (al == 3) {
                    write_bda16(ctx, 0x0450, 0); /* Reset row/col coordinates */
                }
            }
        }
        else if (ah == 0x02) {
            /* Set Cursor Position */
            uint8_t dl = (uint8_t)(dx & 0xFF);
            uint8_t dh = (uint8_t)((dx >> 8) & 0xFF);
            if (ctx->write_mem) {
                ctx->write_mem(ctx->user_data, 0x0450, dl);
                ctx->write_mem(ctx->user_data, 0x0451, dh);
            }
        }
        else if (ah == 0x03) {
            /* Read Cursor Position */
            if (ctx->read_mem) {
                uint8_t dl = ctx->read_mem(ctx->user_data, 0x0450);
                uint8_t dh = ctx->read_mem(ctx->user_data, 0x0451);
                regs.cx = 0x0607;
                regs.dx = (uint32_t)((dh << 8) | dl);
                handled = 1;
            }
        }
        else if (ah == 0x06) {
            /* Clear Window / Scroll Up */
            if (al == 0 && ctx->write_mem) {
                ctx->write_mem(ctx->user_data, 0x0450, 0);
                ctx->write_mem(ctx->user_data, 0x0451, 0);
            }
        }
        else if (ah == 0x0F) {
            /* Get Current Video Mode */
            if (ctx->read_mem) {
                al = ctx->read_mem(ctx->user_data, BDA_VIDEO_MODE);
                uint8_t cols = ctx->read_mem(ctx->user_data, BDA_COLUMNS);
                regs.ax = (uint32_t)((cols << 8) | al);
                regs.bx = 0;
                handled = 1;
            }
        }

        /* Dispatch to VHAL via IOCTL (9 = VDEV_IOCTL_INT10) */
        if (ctx->vdev_ioctl) {
            ctx->vdev_ioctl(ctx->user_data, 9, &regs);
        }
    }
    /* --- INT 11h: Get Equipment List --- */
    else if (int_num == 0x11) {
        ax = read_bda16(ctx, BDA_EQUIP_WORD);
    }
    /* --- INT 12h: Get Conventional Memory Size --- */
    else if (int_num == 0x12) {
        ax = read_bda16(ctx, BDA_MEM_SIZE);
    }
    /* --- INT 13h: Disk Services --- */
    else if (int_num == 0x13) {
        uint8_t ah = (uint8_t)((ax >> 8) & 0xFF);
        if (ah == 0x00 || ah == 0x01) {
            ax = 0;
        } else {
            ax = 0;
        }
    }
    /* --- INT 15h: System Services --- */
    else if (int_num == 0x15) {
        uint8_t ah = (uint8_t)((ax >> 8) & 0xFF);
        if (!is_at) {
            flags |= 0x01; /* Carry set (unsupported) */
        } else {
            if (ah == 0x86) {
                /* Delay CX:DX microseconds */
                uint32_t usec = (cx << 16) | dx;
                ctx->vdev_sleep(ctx->user_data, (int)(usec / 1000));
            } else if (ah == 0x88) {
                /* Get Extended Memory size */
                ax = 1024; /* Return 1MB mock extended memory */
            } else {
                flags |= 0x01;
            }
        }
    }
    /* --- INT 16h: Keyboard Services --- */
    else if (int_num == 0x16) {
        uint8_t ah = (uint8_t)((ax >> 8) & 0xFF);
        if (ah == 0x00 || ah == 0x10) {
            int key = 0;
            while (1) {
                if (ctx->key_buffer != 0) {
                    key = ctx->key_buffer;
                    ctx->key_buffer = 0;
                    break;
                }

                if (ctx->vdev_ioctl) {
                    MockBiosRegs poll_regs = { 0x1100, 0, 0, 0, 0 };
                    if (ctx->vdev_ioctl(ctx->user_data, 11, &poll_regs)) {
                        key = poll_regs.ax & 0xFF;
                        if (key != 0) break;
                    }
                }

                if (ctx->vdev_sleep) ctx->vdev_sleep(ctx->user_data, 5);
            }
            regs.ax = (uint32_t)(key & 0xFF);
            handled = 1;
        } else if (ah == 0x01 || ah == 0x11) {
            if (ctx->key_buffer == 0) {
                if (ctx->vdev_ioctl) {
                    MockBiosRegs poll_regs = { 0x1100, 0, 0, 0, 0 };
                    if (ctx->vdev_ioctl(ctx->user_data, 11, &poll_regs)) {
                        ctx->key_buffer = poll_regs.ax & 0xFF;
                    }
                }
            }
            if (ctx->key_buffer != 0) {
                regs.flags &= ~0x40; /* Clear ZF (key ready) */
                regs.ax = (uint32_t)(ctx->key_buffer & 0xFF);
            } else {
                regs.flags |= 0x40;  /* Set ZF (no key) */
            }
            handled = 1;
        }
    }
    /* --- INT 17h: Printer Services --- */
    else if (int_num == 0x17) {
        uint8_t ah = (uint8_t)((ax >> 8) & 0xFF);
        uint8_t al = (uint8_t)(ax & 0xFF);
        if (ah == 0x00) {
            if (ctx->vdev_ioctl) {
                MockBiosRegs io_regs = { (uint32_t)(0x0E00 | al), 0, 0, 0, 0 };
                ctx->vdev_ioctl(ctx->user_data, 9, &io_regs);
            }
            regs.ax = (uint32_t)((0x90 << 8) | al); /* Status (not busy, selected) */
        } else {
            regs.ax = (0x90 << 8);
        }
        handled = 1;
    }
    /* --- INT 1Ah: Clock Ticks / Time --- */
    else if (int_num == 0x1A) {
        uint8_t ah = (uint8_t)((ax >> 8) & 0xFF);
        if (ah == 0x00) {
            time_t t = time(NULL);
            struct tm tm_buf;
            struct tm *lt;
            lt = platform_localtime(&t, &tm_buf);
            long seconds = lt ? (lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec) : (long)(t % 86400);
            uint32_t ticks = (uint32_t)(seconds * 18.20648);
            cx = (ticks >> 16) & 0xFFFF;
            dx = ticks & 0xFFFF;
            ax = 0;
            if (ctx->write_mem) {
                ctx->write_mem(ctx->user_data, BDA_TIMER_TICKS, ticks & 0xFF);
                ctx->write_mem(ctx->user_data, BDA_TIMER_TICKS + 1, (ticks >> 8) & 0xFF);
                ctx->write_mem(ctx->user_data, BDA_TIMER_TICKS + 2, (ticks >> 16) & 0xFF);
                ctx->write_mem(ctx->user_data, BDA_TIMER_TICKS + 3, (ticks >> 24) & 0xFF);
            }
        } else if (ah == 0x01) {
            /* Set clock ticks */
        } else if (ah == 0x02 || ah == 0x04) {
            if (!is_at) {
                flags |= 0x01;
            } else {
                time_t t = time(NULL);
                struct tm tm_buf;
                struct tm *lt;
                lt = platform_localtime(&t, &tm_buf);
                if (lt) {
                    if (ah == 0x02) {
                        cx = (uint32_t)((((lt->tm_hour / 10) % 10) << 12) | ((lt->tm_hour % 10) << 8) |
                             (((lt->tm_min / 10) % 10) << 4) | (lt->tm_min % 10));
                        dx = (uint32_t)((((lt->tm_sec / 10) % 10) << 12) | ((lt->tm_sec % 10) << 8));
                    } else {
                        int year = lt->tm_year + 1900;
                        int cent = year / 100;
                        year = year % 100;
                        cx = (uint32_t)((((cent / 10) % 10) << 12) | ((cent % 10) << 8) |
                             (((year / 10) % 10) << 4) | (year % 10));
                        int mon = lt->tm_mon + 1;
                        dx = (uint32_t)((((mon / 10) % 10) << 12) | ((mon % 10) << 8) |
                             (((lt->tm_mday / 10) % 10) << 4) | (lt->tm_mday % 10));
                    }
                } else {
                    flags |= 0x01;
                }
            }
        }
    }
    /* --- INT 21h: MS-DOS API services --- */
    else if (int_num == 0x21) {
        uint8_t ah = (uint8_t)((ax >> 8) & 0xFF);
        uint8_t al = (uint8_t)(ax & 0xFF);
        if (ah == 0x09) {
            uint32_t addr = dx;
            while (1) {
                if (ctx->read_mem == NULL) break;
                uint8_t ch = ctx->read_mem(ctx->user_data, addr++);
                if (ch == '$' || ch == 0) break;
                if (ctx->vdev_ioctl) {
                    MockBiosRegs io_regs = { (uint32_t)(0x0E00 | ch), 0, 0, 0, 0 };
                    ctx->vdev_ioctl(ctx->user_data, 9, &io_regs);
                }
            }
        }
        else if (ah == 0x02) {
            if (ctx->vdev_ioctl) {
                MockBiosRegs io_regs = { (uint32_t)(0x0E00 | (dx & 0xFF)), 0, 0, 0, 0 };
                ctx->vdev_ioctl(ctx->user_data, 9, &io_regs);
            }
        }
        else if (ah == 0x01 || ah == 0x08) {
            int key = 0;
            while (1) {
                if (ctx->key_buffer != 0) {
                    key = ctx->key_buffer;
                    ctx->key_buffer = 0;
                    break;
                }
                if (ctx->vdev_ioctl) {
                    MockBiosRegs poll_regs = { 0x1100, 0, 0, 0, 0 };
                    if (ctx->vdev_ioctl(ctx->user_data, 11, &poll_regs)) {
                        key = poll_regs.ax & 0xFF;
                        if (key != 0) break;
                    }
                }
                if (ctx->vdev_sleep) ctx->vdev_sleep(ctx->user_data, 5);
            }
            if (ah == 0x01 && ctx->vdev_ioctl && key != '\r') {
                MockBiosRegs io_regs = { (uint32_t)(0x0E00 | (key & 0xFF)), 0, 0, 0, 0 };
                ctx->vdev_ioctl(ctx->user_data, 9, &io_regs);
            }
            regs.ax = (uint32_t)(key & 0xFF);
            handled = 1;
        }
        else if (ah == 0x0B) {
            if (ctx->key_buffer == 0) {
                if (ctx->vdev_ioctl) {
                    MockBiosRegs poll_regs = { 0x1100, 0, 0, 0, 0 };
                    if (ctx->vdev_ioctl(ctx->user_data, 11, &poll_regs)) {
                        ctx->key_buffer = poll_regs.ax & 0xFF;
                    }
                }
            }
            al = (ctx->key_buffer != 0) ? 0xFF : 0x00;
            ax = (ax & ~0xFF) | al;
        }
        else if (ah == 0x30) {
            ax = 0x0500;
            bx = 0xFF00;
            cx = 0;
        }
        else if (ah == 0x39 || ah == 0x3A || ah == 0x3B || ah == 0x47 || ah == 0x3C || ah == 0x3D || ah == 0x3E || ah == 0x3F || ah == 0x40) {
            char path[256];
            uint32_t addr = dx;
            int i = 0;
            if (ah != 0x3E && ah != 0x3F && ah != 0x40 && ah != 0x47 && ctx->read_mem) {
                while (i < 255) {
                    uint8_t ch = ctx->read_mem(ctx->user_data, addr++);
                    if (ch == '\0') break;
                    path[i++] = ch;
                }
            }
            path[i] = '\0';

            typedef struct { const char *path; char *out_buffer; int out_max; int for_write; } BiosVfsArgs;

            if (ah == 0x39) { /* MKDIR */
                ax = (ctx->vdev_ioctl && ctx->vdev_ioctl(ctx->user_data, 14, path)) ? 0 : 3;
                if (ax == 0) flags &= ~1; else flags |= 1;
            } else if (ah == 0x3A) { /* RMDIR */
                ax = (ctx->vdev_ioctl && ctx->vdev_ioctl(ctx->user_data, 15, path)) ? 0 : 3;
                if (ax == 0) flags &= ~1; else flags |= 1;
            } else if (ah == 0x3B) { /* CHDIR */
                ax = (ctx->vdev_ioctl && ctx->vdev_ioctl(ctx->user_data, 16, path)) ? 0 : 3;
                if (ax == 0) flags &= ~1; else flags |= 1;
            } else if (ah == 0x47) { /* GETCWD */
                if (ctx->vdev_ioctl && ctx->vdev_ioctl(ctx->user_data, 17, path)) {
                    addr = dx;
                    if (ctx->write_mem) {
                        for (int j = 0; path[j]; j++) ctx->write_mem(ctx->user_data, addr++, path[j]);
                        ctx->write_mem(ctx->user_data, addr, 0);
                    }
                    flags &= ~1; ax = 0;
                } else {
                    flags |= 1; ax = 15;
                }
            } else if (ah == 0x3C) { /* CREATE FILE */
                BiosVfsArgs vfs = { path, path, sizeof(path), 1 };
                if (ctx->vdev_ioctl && ctx->vdev_ioctl(ctx->user_data, 13, &vfs)) {
                    int handle = -1;
                    for (int j = 0; j < 20; j++) if (!ctx->dos_handles[j]) { handle = j; break; }
                    if (handle >= 0) {
                        ctx->dos_handles[handle] = fopen(vfs.out_buffer, "wb+");
                        if (ctx->dos_handles[handle]) { ax = handle; flags &= ~1; }
                        else { ax = 3; flags |= 1; }
                    } else { ax = 4; flags |= 1; }
                } else { ax = 3; flags |= 1; }
            } else if (ah == 0x3D) { /* OPEN FILE */
                BiosVfsArgs vfs = { path, path, sizeof(path), 0 };
                if (ctx->vdev_ioctl && ctx->vdev_ioctl(ctx->user_data, 13, &vfs)) {
                    int handle = -1;
                    for (int j = 0; j < 20; j++) if (!ctx->dos_handles[j]) { handle = j; break; }
                    if (handle >= 0) {
                        ctx->dos_handles[handle] = fopen(vfs.out_buffer, "rb+");
                        if (!ctx->dos_handles[handle]) ctx->dos_handles[handle] = fopen(vfs.out_buffer, "rb");
                        if (ctx->dos_handles[handle]) { ax = handle; flags &= ~1; }
                        else { ax = 2; flags |= 1; }
                    } else { ax = 4; flags |= 1; }
                } else { ax = 2; flags |= 1; }
            } else if (ah == 0x3E) { /* CLOSE */
                int h = bx & 0xFF;
                if (h < 20 && ctx->dos_handles[h]) {
                    fclose((FILE*)ctx->dos_handles[h]);
                    ctx->dos_handles[h] = NULL;
                    ax = 0; flags &= ~1;
                } else { ax = 6; flags |= 1; }
            } else if (ah == 0x3F) { /* READ */
                int h = bx & 0xFF;
                if (h < 20 && ctx->dos_handles[h]) {
                    char buf[1024];
                    int to_read = cx > 1024 ? 1024 : cx;
                    size_t read_bytes = fread(buf, 1, to_read, (FILE*)ctx->dos_handles[h]);
                    if (ctx->write_mem) {
                        for (size_t j = 0; j < read_bytes; j++) ctx->write_mem(ctx->user_data, (uint32_t)(dx + j), buf[j]);
                    }
                    ax = (uint32_t)read_bytes; flags &= ~1;
                } else { ax = 6; flags |= 1; }
            } else if (ah == 0x40) { /* WRITE */                int h = bx & 0xFF;
                if (h < 20 && ctx->dos_handles[h]) {
                    char buf[1024];
                    int to_write = cx > 1024 ? 1024 : cx;
                    if (ctx->read_mem) {
                        for (int j = 0; j < to_write; j++) buf[j] = ctx->read_mem(ctx->user_data, (uint32_t)(dx + j));
                    }
                    size_t written = fwrite(buf, 1, to_write, (FILE*)ctx->dos_handles[h]);
                    ax = (uint32_t)written; flags &= ~1;
                } else { ax = 6; flags |= 1; }
            }
        }
        else if (ah == 0x48) {
            uint32_t size_paras = bx;
            if (ctx->vdev_ioctl) {
                ax = ctx->vdev_ioctl(ctx->user_data, 18, (void*)(size_t)size_paras);
                if (ax) flags &= ~1; else { flags |= 1; ax = 8; }
            } else { flags |= 1; ax = 8; }
        } else if (ah == 0x49) {
            uint32_t segment = bx;
            if (ctx->vdev_ioctl) {
                ax = ctx->vdev_ioctl(ctx->user_data, 19, (void*)(size_t)segment);
                flags &= ~1;
            } else { flags |= 1; ax = 9; }
        } else if (ah == 0x4A) {
            uint32_t new_size_paras = bx;
            if (ctx->vdev_ioctl) {
                ax = ctx->vdev_ioctl(ctx->user_data, 20, (void*)(size_t)new_size_paras);
                if (ax) flags &= ~1; else { flags |= 1; ax = 8; }
            } else { flags |= 1; ax = 8; }
        }
        else if (ah == 0x4C) {
            char term_msg[64];
            snprintf(term_msg, sizeof(term_msg), "\r\n[DOS] Program terminated with exit code %d\r\n", al);
            for (int i = 0; term_msg[i] != '\0'; i++) {
                if (ctx->vdev_ioctl) {
                    MockBiosRegs io_regs = { (uint32_t)(0x0E00 | (uint8_t)term_msg[i]), 0, 0, 0, 0 };
                    ctx->vdev_ioctl(ctx->user_data, 9, &io_regs);
                }
            }
        }
    }
    /* --- INT 23h: MS-DOS Ctrl-C Exit Handler --- */
    else if (int_num == 0x23) {
        char term_msg[] = "\r\n[DOS] Program aborted (Ctrl-C / Break)\r\n";
        for (int i = 0; term_msg[i] != '\0'; i++) {
            if (ctx->vdev_ioctl) {
                MockBiosRegs io_regs = { (uint32_t)(0x0E00 | (uint8_t)term_msg[i]), 0, 0, 0, 0 };
                ctx->vdev_ioctl(ctx->user_data, 9, &io_regs);
            }
        }
    }

    if (handled) {
        ax = regs.ax;
        bx = regs.bx;
        cx = regs.cx;
        dx = regs.dx;
        flags = regs.flags;
    }
    if (ctx->set_registers) {
        ctx->set_registers(ctx->user_data, ax, bx, cx, dx, flags);
    }
}

/**
 * @brief Convert a MockBiosModel enum to its string identifier.
 *
 * What it does: Returns a static string name corresponding to the enum value.
 * Why it exists: Used by the MEMMAP$ built-in function to query the active model name.
 * Why it works this way: Simple mapping using a switch statement.
 * Assumptions: The enum matches the model values.
 * Portability concerns: None.
 */
const char *mock_bios_model_to_string(MockBiosModel model) {
    switch (model) {
        case BIOS_MODEL_MSDOS: return "MSDOS";
        case BIOS_MODEL_PC:    return "IBMPC";
        case BIOS_MODEL_PCJR:  return "PCJR";
        case BIOS_MODEL_XT:    return "PCXT";
        case BIOS_MODEL_AT:    return "PCAT";
        case BIOS_MODEL_ATARI: return "ATARI";
        case BIOS_MODEL_C64:   return "C64";
        case BIOS_MODEL_VIC20: return "VIC20";
        case BIOS_MODEL_APPLE2: return "APPLE2";
        case BIOS_MODEL_TRS80: return "TRS80";
        case BIOS_MODEL_TANDY: return "TANDY";
        default:               return "NONE";
    }
}

/**
 * @brief Parse a model string identifier into its corresponding MockBiosModel enum.
 *
 * What it does: Matches a string name case-insensitively to its model enum.
 * Why it exists: Used by the BIOS statement when switching models.
 * Why it works this way: Sequentially compares using strcasecmp.
 * Assumptions: Input string is null-terminated.
 * Portability concerns: None.
 */
MockBiosModel mock_bios_model_from_string(const char *name) {
    if (!name) return BIOS_MODEL_NONE;
    if (strcasecmp(name, "NONE") == 0) return BIOS_MODEL_NONE;
    if (strcasecmp(name, "MSDOS") == 0 || strcasecmp(name, "DOS") == 0) return BIOS_MODEL_MSDOS;
    if (strcasecmp(name, "IBMPC") == 0 || strcasecmp(name, "PC") == 0) return BIOS_MODEL_PC;
    if (strcasecmp(name, "PCJR") == 0) return BIOS_MODEL_PCJR;
    if (strcasecmp(name, "PCXT") == 0 || strcasecmp(name, "XT") == 0) return BIOS_MODEL_XT;
    if (strcasecmp(name, "PCAT") == 0 || strcasecmp(name, "AT") == 0) return BIOS_MODEL_AT;
    if (strcasecmp(name, "ATARI") == 0 || strcasecmp(name, "ATARI8") == 0) return BIOS_MODEL_ATARI;
    if (strcasecmp(name, "C64") == 0) return BIOS_MODEL_C64;
    if (strcasecmp(name, "VIC20") == 0) return BIOS_MODEL_VIC20;
    if (strcasecmp(name, "APPLE2") == 0 || strcasecmp(name, "APPLE") == 0) return BIOS_MODEL_APPLE2;
    if (strcasecmp(name, "TRS80") == 0) return BIOS_MODEL_TRS80;
    if (strcasecmp(name, "TANDY") == 0) return BIOS_MODEL_TANDY;
    return BIOS_MODEL_NONE;
}

