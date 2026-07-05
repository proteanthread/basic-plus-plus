/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mock_bios_core.c
 * Subsystem: Switchable IBM PC BIOS Emulator
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Emulates IBM PC BIOS interrupts (INT 10h, 16h, 21h).
 *
 * 2. WHAT TO EXPECT:
 *    Interfaces with BDA/IVT mapping hardware calls.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Emulated ports, system timer tick rules.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    IVT/BDA hardware offset rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If system crashes, check BDA values configuration.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE MOCK BIOS SUBSYSTEM
 * File: mock_bios_core.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    A completely portable C implementation of an IBM PC, XT, AT, and PCjr
 *    mock BIOS system. It simulates the BDA (BIOS Data Area), IVT (Interrupt
 *    Vector Table), ROM signatures, hardware ports (PPI, PIT, CGA, PIC, CMOS),
 *    and standard x86 BIOS interrupts.
 *
 * 2. PORTABILITY CONCERNS:
 *    Has zero dependencies on any BASIC++ interpreter headers or structures.
 *    Relies entirely on ANSI C17 standard headers and user-provided callback
 *    context structures. Can be directly reused in other emulators, compilers,
 *    or bare-metal projects.
 *
 * 3. MEMORY MANAGEMENT:
 *    Zero dynamic allocations. Emulates segmented memory access dynamically
 *    through client-defined memory read/write callback functions.
 * ===================================================================== */

#include "mock_bios_core.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

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

/* Direct memory utility function to set 16-bit values in target buffer if size permits */
static void write16_direct(uint8_t *mem, size_t size, uint32_t addr, uint16_t val)
{
    if (addr + 1 < size) {
        mem[addr] = (uint8_t)(val & 0xFF);
        mem[addr + 1] = (uint8_t)((val >> 8) & 0xFF);
    }
}

/* Helper to read BDA using callback context */
static uint16_t read_bda16(MockBiosContext *ctx, uint32_t addr)
{
    uint8_t low = ctx->read_mem(ctx->user_data, addr);
    uint8_t high = ctx->read_mem(ctx->user_data, addr + 1);
    return (uint16_t)((high << 8) | low);
}

/* Helper to write BDA using callback context */
static void write_bda16(MockBiosContext *ctx, uint32_t addr, uint16_t val)
{
    ctx->write_mem(ctx->user_data, addr, (uint8_t)(val & 0xFF));
    ctx->write_mem(ctx->user_data, addr + 1, (uint8_t)((val >> 8) & 0xFF));
}

/* Initialize mock BIOS memory structures (IVT, BDA, signatures) */
void mock_bios_init_mem(MockBiosContext *ctx, uint8_t *mem, size_t size, MockBiosModel model)
{
    ctx->model = model;
    ctx->key_buffer = 0;
    ctx->cmos_idx = 0;
    memset(ctx->cmos_regs, 0, sizeof(ctx->cmos_regs));
    ctx->ppi_port_b = 0;
    ctx->pit_counter = 0;
    ctx->crtc_index = 0;
    memset(ctx->crtc_regs, 0, sizeof(ctx->crtc_regs));

    if (model == BIOS_MODEL_NONE) {
        if (size > 0) memset(mem, 0, size);
        return;
    }

    /* 1. Initialize IVT (Interrupt Vector Table) pointing to ROM segment 0xF000 */
    for (int i = 0; i < 256; i++) {
        uint32_t ivt_addr = i * 4;
        uint16_t ip_offset = (uint16_t)(0x1000 + (i * 16));
        uint16_t cs_segment = 0xF000;
        write16_direct(mem, size, ivt_addr, ip_offset);
        write16_direct(mem, size, ivt_addr + 2, cs_segment);
    }

    /* 2. Initialize BDA (BIOS Data Area) base settings */
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
        if (model == BIOS_MODEL_PCJR) {
            mem[BDA_VIDEO_MODE] = 0x01;                 /* Mode 1 (PCjr 40x25) */
            write16_direct(mem, size, BDA_COLUMNS, 40); /* 40 columns */
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

/* Emulate port reads (INP instruction) */
uint8_t mock_bios_in(MockBiosContext *ctx, uint16_t port)
{
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
        struct tm *lt = localtime(&t);
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

/* Emulate port writes (OUT instruction) */
void mock_bios_out(MockBiosContext *ctx, uint16_t port, uint8_t val)
{
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

/* Emulate BIOS Interrupts (INT instruction) */
void mock_bios_interrupt(MockBiosContext *ctx, uint8_t int_num)
{
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
            /* Emulated sector reads/writes just return success */
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
                MockBiosRegs io_regs = { (0x0E00 | al), 0, 0, 0, 0 };
                ctx->vdev_ioctl(ctx->user_data, 9, &io_regs); /* Map printer to INT 10h TTY for now */
            }
            regs.ax = (0x90 << 8) | al; /* Status (not busy, selected) */
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
            struct tm *lt = localtime(&t);
            long seconds = lt ? (lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec) : (long)(t % 86400);
            uint32_t ticks = (uint32_t)(seconds * 18.20648);
            cx = (ticks >> 16) & 0xFFFF;
            dx = ticks & 0xFFFF;
            ax = 0;
            ctx->write_mem(ctx->user_data, BDA_TIMER_TICKS, ticks & 0xFF);
            ctx->write_mem(ctx->user_data, BDA_TIMER_TICKS + 1, (ticks >> 8) & 0xFF);
            ctx->write_mem(ctx->user_data, BDA_TIMER_TICKS + 2, (ticks >> 16) & 0xFF);
            ctx->write_mem(ctx->user_data, BDA_TIMER_TICKS + 3, (ticks >> 24) & 0xFF);
        } else if (ah == 0x01) {
            /* Set system clock ticks (mock success) */
        } else if (ah == 0x02 || ah == 0x04) {
            if (!is_at) {
                flags |= 0x01; /* RTC not on PC/XT */
            } else {
                time_t t = time(NULL);
                struct tm *lt = localtime(&t);
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
                    MockBiosRegs io_regs = { (0x0E00 | ch), 0, 0, 0, 0 };
                    ctx->vdev_ioctl(ctx->user_data, 9, &io_regs);
                }
            }
        }
        else if (ah == 0x02) {
            if (ctx->vdev_ioctl) {
                MockBiosRegs io_regs = { (0x0E00 | (dx & 0xFF)), 0, 0, 0, 0 };
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
                MockBiosRegs io_regs = { (0x0E00 | (key & 0xFF)), 0, 0, 0, 0 };
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
        else if (ah == 0x4C) {
            char term_msg[64];
            #ifdef _MSC_VER
            sprintf_s(term_msg, sizeof(term_msg), "\r\n[DOS] Program terminated with exit code %d\r\n", al);
            #else
            sprintf(term_msg, "\r\n[DOS] Program terminated with exit code %d\r\n", al);
            #endif
            for (int i = 0; term_msg[i] != '\0'; i++) {
                if (ctx->vdev_ioctl) {
                    MockBiosRegs io_regs = { (0x0E00 | (uint8_t)term_msg[i]), 0, 0, 0, 0 };
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
                MockBiosRegs io_regs = { (0x0E00 | (uint8_t)term_msg[i]), 0, 0, 0, 0 };
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
