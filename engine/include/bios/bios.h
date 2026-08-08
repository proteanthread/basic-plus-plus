/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bios.h
 * @brief Master BIOS hardware abstraction and interrupt vector table header declarations.
 *
 * 1. WHAT IT DOES:
 *    Declares the master hardware abstraction interface for IBM PC/XT/AT/PCjr BIOS emulation:
 *    `bios_init()`, `bios_dispatch_interrupt()`, `bios_read_port()`, `bios_write_port()`, and BDA (BIOS Data Area) registers.
 *
 * 2. WHY IT EXISTS:
 *    Provides the public C API header for micro-libraries and statement handlers calling low-level hardware or BIOS interrupt services.
 *
 * 3. WHY IT WORKS THIS WAY:
 *    Defines `BppRegisters` (16-bit register struct containing AX, BX, CX, DX, SI, DI, BP, DS, ES, FLAGS) and interrupt dispatch function prototypes.
 *
 * 4. DEPENDENCIES & COMPILATION:
 *    - Required Headers: `types/types.h`, `types/errors.h`, `<stdint.h>`, `<stdbool.h>`
 *    - CMake Target: Header surface for `bios` micro-library target in `engine/CMakeLists.txt`.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 *    - Included in `libbasicpp` (`baspp`).
 *    - Excluded from `libbasicpp_lite` (`bpp`/`bs`) when BIOS emulation is disabled.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 *    - To add new BIOS interrupt prototypes (e.g. `bios_int13_disk_read()`): declare function prototypes in `bios.h` and implement in `bios.c`.
 *    - To add custom I/O port ranges: update `PORT_MIN` / `PORT_MAX` macro bounds.
 *
 * 7. WHAT CANNOT BE CHANGED:
 *    - `BppRegisters` struct field layout ordering (must match 8086 register push/pop conventions).
 *    - Interrupt vector range (`0x00`..`0xFF`).
 *
 * 8. WHAT TO EXPECT:
 *    - Include self-sufficiency (compiles independently when included alone in an empty `.c` file).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 *    - Ensure `<stdint.h>` is included for `uint8_t`, `uint16_t`, `uint32_t`.
 *    - Verify `#ifndef BIOS_H` header include guard.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 *     - Included by statement handlers (`bios.c`, `peek.c`, `poke.c`).
 *
 * 11. PORTABILITY & C17 CONCERNS:
 *     - Strict C17 compliance (`-std=c17`).
 *     - Pointer-to-integer casts prohibited; uses `uintptr_t` for memory addresses.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 *     - Prerequisite C Source Files: `engine/src/bios/bios.c` (port I/O implementation), `engine/src/bios/bios_pc.c` (PC interrupt implementation).
 *     - Prerequisite Header Surfaces: `engine/include/bios/bios_types.h`, `engine/include/types/types.h`, `engine/include/types/errors.h`.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bios.h
 * @brief Public header for the authentic IBM PC/XT/AT/PCjr BIOS C17 micro-library.
 *
 * =========================================================================
 * SECTION 1: ARCHITECTURAL OVERVIEW, DESIGN GOALS, AND MEMORY LAYOUTS
 * =========================================================================
 * - What it does: Implements a 100% C17-compliant, authentic IBM PC (5150),
 *   XT (5160), AT (5170), and PCjr (4860) BIOS compatibility engine. It accurately
 *   emulates the Interrupt Vector Table (IVT), BIOS Data Area (BDA), video RAM (VRAM),
 *   hardware I/O ports (8254 PIT, 8259 PIC, 6845 CRTC, 8042 KBD, CMOS RTC 0x70/0x71,
 *   SN76496 PSG 0xC0, 8237 DMA, AdLib FM 0x388, Sound Blaster DSP 0x220, Game Port 0x201),
 *   and BIOS/DOS system service interrupts (INT 10h, 11h, 12h, 13h, 14h, 15h, 16h, 17h,
 *   1Ah, 21h, 2Fh, 33h, 67h).
 *
 * - Why it exists: To provide a standalone, highly portable C17 BIOS micro-library
 *   that powers BASIC++ PEEK, POKE, INP, OUT, SYS, USR, and THROTTLE statements
 *   while remaining completely decoupled for external C17 projects or UEFI CSM
 *   legacy boot engines.
 *
 * - 1MB Real-Mode Memory Layout Diagram:
 *   +-------------------------------------------------------------------+
 *   | Physical Address Range | Logical Block Description                |
 *   +------------------------+------------------------------------------+
 *   | 0x00000 - 0x003FF      | Interrupt Vector Table (IVT, 256 x 4B)   |
 *   | 0x00400 - 0x004FF      | BIOS Data Area (BDA, 256 bytes)          |
 *   | 0x00500 - 0x005FF      | DOS / BASIC System Scratchpad            |
 *   | 0x00600 - 0x9FFFF      | Conventional User Memory Area (640 KB)   |
 *   | 0xA0000 - 0xAFFFF      | EGA / VGA Framebuffer (64 KB)            |
 *   | 0xB0000 - 0xB7FFF      | MDA Monochrome Text Buffer (32 KB)       |
 *   | 0xB8000 - 0xBFFFF      | CGA Color Text/Graphics Buffer (32 KB)   |
 *   | 0xC0000 - 0xC7FFF      | Video BIOS Expansion ROM (32 KB)         |
 *   | 0xC8000 - 0xDFFFF      | Hard Disk / Network Adapter Option ROMs  |
 *   | 0xE0000 - 0xEFFFF      | LIM EMS 4.0 Page Frame / System Expansion|
 *   | 0xF6000 - 0xFDFFF      | IBM Cassette BASIC ROM Sockets (32 KB)   |
 *   | 0xFE000 - 0xFFFFF      | IBM System BIOS ROM & Reset Vector (8KB)|
 *   +-------------------------------------------------------------------+
 *
 * =========================================================================
 * SECTION 2: IBM HISTORICAL ROM PART-NUMBER & EPROM SOCKET MATRIX
 * =========================================================================
 * Native IBM PC 5150 motherboards use 2364 ROM sockets (24-pin, 8KB) for U33
 * and U29-U32. 2364 pinouts differ from standard 2764 / 27256 EPROMs:
 *
 *   2364 Socket Pinout:            2764 EPROM Pinout:
 *       +--\/--+                       +--\/--+
 *   A7  | 1  24| VCC (+5V)         VPP | 1  28| VCC (+5V)
 *   A6  | 2  23| A8                A12 | 2  27| /PGM
 *   A5  | 3  22| A9                A7  | 3  26| NC
 *   A4  | 4  21| /CS1 (A13)        A6  | 4  25| A8
 *   A3  | 5  20| /CS2              A5  | 5  24| A9
 *   A2  | 6  19| A10               A4  | 6  23| A11
 *   A1  | 7  18| A11               A3  | 7  22| /OE
 *   A0  | 8  17| D7                A2  | 8  21| A10
 *   D0  | 9  16| D6                A1  | 9  20| /CE
 *   D1  |10  15| D5                A0  |10  19| D7
 *   D2  |11  14| D4                D0  |11  18| D6
 *   GND |12  13| D3                D1  |12  17| D5
 *       +------+                   D2  |13  16| D4
 *                                  GND |14  15| D3
 *                                      +------+
 *
 * Replacement Options for IBM Socket U33:
 * 1. MCM68766 EPROM: Direct 24-pin drop-in replacement.
 * 2. 2564 EPROM: Direct pinout compatible 8KB EPROM.
 * 3. 2764 / 27C64 EPROM: Requires 28-pin to 24-pin socket adapter.
 * 4. Winbond W27E257 EEPROM: Requires 28-pin adapter with quadrupled 8KB images.
 *
 * =========================================================================
 * SECTION 3: UEFI CSM (COMPATIBILITY SUPPORT MODULE) HANDOFF GUIDE
 * =========================================================================
 * To use `libbios` in a modern 64-bit UEFI environment as a legacy boot CSM:
 * 1. Allocate a 1MB contiguous physical memory buffer below 1MB (or use
 *    Identity Paging in UEFI page tables).
 * 2. Initialize `libbios` context using `bios_create(BIOS_MODEL_IBM_AT)`.
 * 3. Bind UEFI GOP (Graphics Output Protocol) framebuffer to INT 10h VBE handler.
 * 4. Bind UEFI Simple Text Input to INT 16h / BDA keyboard buffer.
 * 5. Bind UEFI Block I/O protocol to INT 13h EDD 3.0 LBA48 GPT disk service.
 * 6. Anchor ACPI 2.0 RSDP (`"RSD PTR "`) and SMBIOS 3.0 (`"_SM3_"`) structures at
 *    physical offset `0xE0000` - `0xFFFFF`.
 *
 * =========================================================================
 * SECTION 4: DEVELOPER MAINTENANCE, INVARIANTS, AND PORTABILITY CONCERNS
 * =========================================================================
 * - Strict C17 Compliance (-std=c17). No C++ extensions or compiler pragmas.
 * - Portable 64-bit pointer safety (`intptr_t` / `uintptr_t`).
 * - Zero-initialization by default for all dynamic contexts.
 */

#ifndef ENGINE_BIOS_BIOS_H
#define ENGINE_BIOS_BIOS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Hardware Model Classification
 * ------------------------------------------------------------------------- */
typedef enum {
    BIOS_MODEL_IBM_PC   = 0, /* IBM PC 5150 (Model 0xFF) */
    BIOS_MODEL_IBM_XT   = 1, /* IBM PC/XT 5160 (Model 0xFE) */
    BIOS_MODEL_IBM_AT   = 2, /* IBM PC/AT 5170 (Model 0xFC) */
    BIOS_MODEL_IBM_PCJR = 3  /* IBM PCjr 4860 (Model 0xFD) */
} BiosModel;

/* -------------------------------------------------------------------------
 * Fine-Grained Historical IBM BIOS Release Revisions
 * ------------------------------------------------------------------------- */
typedef enum {
    /* IBM PC 5150 Revisions */
    BIOS_REV_PC_1981_04_24 = 0, /* 5150 Rev 1 (Part # 5700051, Date 04/24/81) */
    BIOS_REV_PC_1981_10_19 = 1, /* 5150 Rev 2 (Part # 5700671, Date 10/19/81) */
    BIOS_REV_PC_1982_08_16 = 2, /* 5150 Rev 3 (Part # 5000024, Date 08/16/82) */
    BIOS_REV_PC_1982_10_27 = 3, /* 5150 Rev 4 (Part # 1501476, Date 10/27/82) */

    /* IBM PC/XT 5160 Revisions */
    BIOS_REV_XT_1982_11_08 = 4, /* 5160 Rev 1 (Part # 1501512, Date 11/08/82) */
    BIOS_REV_XT_1986_01_10 = 5, /* 5160 Rev 2 (Part # 62X0819, Date 01/10/86) */
    BIOS_REV_XT_1986_05_09 = 6, /* 5160 Rev 3 (Part # 62X0851, Date 05/09/86) */

    /* IBM PC/AT 5170 Revisions */
    BIOS_REV_AT_1984_01_10 = 7, /* 5170 Rev 1 (Part # 1501529, Date 01/10/84, 6MHz) */
    BIOS_REV_AT_1985_06_10 = 8, /* 5170 Rev 2 (Part # 6181028, Date 06/10/85, 6MHz) */
    BIOS_REV_AT_1985_11_15 = 9, /* 5170 Rev 3 (Part # 61F9018, Date 11/15/85, 8MHz) */

    /* IBM PCjr 4860 Revision */
    BIOS_REV_PCJR_1983_06_01 = 10 /* 4860 Rev 1 (Part # 1501401, Date 06/01/83) */
} BiosRevision;

/* -------------------------------------------------------------------------
 * Speed-Accuracy Clock Throttling Modes
 * ------------------------------------------------------------------------- */
typedef enum {
    BIOS_CLOCK_AUTOMATIC    = 0, /* Default: Domain-scoped automatic throttling */
    BIOS_CLOCK_CONTINUOUS   = 1, /* Continuous clock pacing across all code */
    BIOS_CLOCK_UNRESTRICTED = 2  /* No throttling (maximum host CPU speed) */
} BiosClockMode;

/* -------------------------------------------------------------------------
 * x86 CPU Registers Structure for Interrupt Handling
 * ------------------------------------------------------------------------- */
typedef struct {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
    uint16_t si;
    uint16_t di;
    uint16_t bp;
    uint16_t sp;
    uint16_t flags; /* Carry=0x0001, Zero=0x0040, Interrupt=0x0200 */
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t ss;
} BiosRegs;

/* -------------------------------------------------------------------------
 * BIOS Data Area (BDA) Structure at Offset 0x00400 (256 bytes)
 * ------------------------------------------------------------------------- */
#pragma pack(push, 1)
typedef struct {
    uint16_t com_port[4];        /* 0x400: RS232 I/O port addresses (COM1-COM4) */
    uint16_t lpt_port[4];        /* 0x408: Parallel printer I/O ports (LPT1-LPT4) */
    uint16_t equipment_word;     /* 0x410: Installed hardware equipment flags */
    uint8_t  post_err_flag;      /* 0x412: POST error code flag */
    uint16_t memory_size_kb;     /* 0x413: Conventional RAM size in KB (640) */
    uint8_t  scratch_pad;        /* 0x415: Reserved scratchpad */
    uint8_t  kbd_flag_1;         /* 0x417: Keyboard Shift/Ctrl/Alt status flags */
    uint8_t  kbd_flag_2;         /* 0x418: Keyboard NumLock/CapsLock/ScrollLock flags */
    uint8_t  alt_keypad;         /* 0x419: Alt-Keypad entry workspace */
    uint16_t kbd_buf_head;       /* 0x41A: Keyboard buffer head pointer (offset from 0x400) */
    uint16_t kbd_buf_tail;       /* 0x41C: Keyboard buffer tail pointer (offset from 0x400) */
    uint16_t kbd_buffer[16];     /* 0x41E: Keyboard 16-word circular buffer (32 bytes) */
    uint8_t  diskette_recal;     /* 0x43E: Diskette recalibration status */
    uint8_t  diskette_motor;     /* 0x43F: Diskette motor status */
    uint8_t  diskette_timeout;   /* 0x440: Diskette motor timeout counter */
    uint8_t  diskette_last_err;  /* 0x441: Diskette status return code */
    uint8_t  fdc_status[7];      /* 0x442: FDC controller status bytes */
    uint8_t  video_mode;         /* 0x449: Current display mode ID */
    uint16_t video_cols;         /* 0x44A: Text columns per line (40 or 80) */
    uint16_t video_page_size;    /* 0x44C: Display page size in bytes */
    uint16_t video_page_off;     /* 0x44E: Current page start offset in VRAM */
    uint16_t cursor_pos[8];      /* 0x450: Cursor row/col for 8 display pages */
    uint16_t cursor_lines;       /* 0x460: Cursor start/end scan lines */
    uint8_t  active_page;        /* 0x462: Active display page number */
    uint16_t crtc_base_port;     /* 0x463: CRTC controller base port (0x3D4 or 0x3B4) */
    uint8_t  crtc_mode_reg;      /* 0x465: Current mode select register value */
    uint8_t  crtc_color_palette; /* 0x466: Current CGA color palette mask */
    uint32_t timer_ticks;        /* 0x46C: Daily timer tick counter (18.2 ticks/sec) */
    uint8_t  timer_overflow;     /* 0x470: Daily timer 24-hour rollover flag */
    uint8_t  ctrl_break_flag;    /* 0x471: Ctrl-Break flag bit */
    uint16_t reset_flag;         /* 0x472: Soft reset flag (0x1234 = skip POST) */
    uint8_t  fixed_disk_status;  /* 0x474: Hard disk status code */
    uint8_t  num_fixed_disks;    /* 0x475: Number of fixed disk drives connected */
    uint8_t  fixed_disk_control; /* 0x476: Fixed disk control byte */
    uint8_t  fixed_disk_port;    /* 0x477: Fixed disk I/O port offset */
    uint8_t  print_lpt_timeout[4];/* 0x478: LPT printer timeout values */
    uint8_t  rs232_com_timeout[4];/* 0x47C: COM serial timeout values */
    uint16_t kbd_start_off;      /* 0x480: Keyboard buffer start offset */
    uint16_t kbd_end_off;        /* 0x482: Keyboard buffer end offset */
    uint8_t  video_rows;         /* 0x484: Screen character rows - 1 (EGA/VGA) */
    uint16_t char_height;        /* 0x485: Character matrix height in scan lines */
    uint8_t  video_options;      /* 0x487: Video control options flag */
    uint8_t  feature_switches;   /* 0x488: Feature switches mask */
    uint8_t  video_flags_1;      /* 0x489: Video display combination flags */
    uint8_t  pcjr_video_flags;   /* 0x48A: PCjr specific video mode flags */
    uint8_t  reserved_bda[117];  /* 0x48B-0x4FF: Extended BIOS Data Area workspace */
} BiosDataArea;
#pragma pack(pop)

/* -------------------------------------------------------------------------
 * Memory Provider Hook Interface
 * ------------------------------------------------------------------------- */
typedef uint8_t (*BiosMemReadFn)(void* user_data, uint32_t phys_addr);
typedef void    (*BiosMemWriteFn)(void* user_data, uint32_t phys_addr, uint8_t value);

typedef struct {
    void*           user_data;
    BiosMemReadFn   read_u8;
    BiosMemWriteFn  write_u8;
    uint8_t*        flat_1mb_ptr; /* Optional 1MB contiguous block pointer */
} BiosMemoryMap;

/* -------------------------------------------------------------------------
 * Custom Interrupt Handler Function Prototype
 * ------------------------------------------------------------------------- */
typedef struct BiosContext BiosContext;
typedef bool (*BiosIntHandlerFn)(BiosContext* ctx, uint8_t int_num, BiosRegs* regs, void* user_data);

/* -------------------------------------------------------------------------
 * Public API Functions
 * ------------------------------------------------------------------------- */

BiosContext* bios_create(BiosModel model);
void        bios_destroy(BiosContext* ctx);
bool        bios_init(BiosContext* ctx);

void        bios_set_model(BiosContext* ctx, BiosModel model);
BiosModel   bios_get_model(const BiosContext* ctx);

void        bios_set_revision(BiosContext* ctx, BiosRevision revision);
BiosRevision bios_get_revision(const BiosContext* ctx);
const char* bios_get_part_number(const BiosContext* ctx);

void          bios_set_clock_mode(BiosContext* ctx, BiosClockMode mode);
BiosClockMode bios_get_clock_mode(const BiosContext* ctx);
void          bios_set_clock_freq(BiosContext* ctx, double mhz);
double        bios_get_clock_freq(const BiosContext* ctx);

void        bios_set_memory_map(BiosContext* ctx, const BiosMemoryMap* map);
BiosDataArea* bios_get_bda(BiosContext* ctx);

uint8_t     bios_peek(BiosContext* ctx, uint32_t addr);
void        bios_poke(BiosContext* ctx, uint32_t addr, uint8_t val);
void        bios_poke_raw(BiosContext* ctx, uint32_t addr, uint8_t val);

uint8_t     bios_inp(BiosContext* ctx, uint16_t port);
void        bios_out(BiosContext* ctx, uint16_t port, uint8_t val);

void        bios_post_code(BiosContext* ctx, uint8_t code);

bool        bios_register_interrupt(BiosContext* ctx, uint8_t int_num, BiosIntHandlerFn handler, void* user_data);
bool        bios_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs);

#ifdef __cplusplus
}
#endif

#endif /* ENGINE_BIOS_BIOS_H */
