// FILENAME: bios.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, libhardware, libkernel
// NEEDS: platform, memory
// Implements virtual BIOS interrupt and hardware emulation for bios.
//
// ---- Includes ----

#ifndef ENGINE_BIOS_BIOS_H
#define ENGINE_BIOS_BIOS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------
// Hardware Model Classification
// -------------------------------------------------------------------------
typedef enum {
    BIOS_MODEL_IBM_PC   = 0, // IBM PC 5150 (Model 0xFF)
    BIOS_MODEL_IBM_XT   = 1, // IBM PC/XT 5160 (Model 0xFE)
    BIOS_MODEL_IBM_AT   = 2, // IBM PC/AT 5170 (Model 0xFC)
    BIOS_MODEL_IBM_PCJR = 3  // IBM PCjr 4860 (Model 0xFD)
} BiosModel;

// -------------------------------------------------------------------------
// Fine-Grained Historical IBM BIOS Release Revisions
// -------------------------------------------------------------------------
typedef enum {
    // IBM PC 5150 Revisions
    BIOS_REV_PC_1981_04_24 = 0, // 5150 Rev 1 (Part # 5700051, Date 04/24/81)
    BIOS_REV_PC_1981_10_19 = 1, // 5150 Rev 2 (Part # 5700671, Date 10/19/81)
    BIOS_REV_PC_1982_08_16 = 2, // 5150 Rev 3 (Part # 5000024, Date 08/16/82)
    BIOS_REV_PC_1982_10_27 = 3, // 5150 Rev 4 (Part # 1501476, Date 10/27/82)

    // IBM PC/XT 5160 Revisions
    BIOS_REV_XT_1982_11_08 = 4, // 5160 Rev 1 (Part # 1501512, Date 11/08/82)
    BIOS_REV_XT_1986_01_10 = 5, // 5160 Rev 2 (Part # 62X0819, Date 01/10/86)
    BIOS_REV_XT_1986_05_09 = 6, // 5160 Rev 3 (Part # 62X0851, Date 05/09/86)

    // IBM PC/AT 5170 Revisions
    BIOS_REV_AT_1984_01_10 = 7, // 5170 Rev 1 (Part # 1501529, Date 01/10/84, 6MHz)
    BIOS_REV_AT_1985_06_10 = 8, // 5170 Rev 2 (Part # 6181028, Date 06/10/85, 6MHz)
    BIOS_REV_AT_1985_11_15 = 9, // 5170 Rev 3 (Part # 61F9018, Date 11/15/85, 8MHz)

    // IBM PCjr 4860 Revision
    BIOS_REV_PCJR_1983_06_01 = 10 // 4860 Rev 1 (Part # 1501401, Date 06/01/83)
} BiosRevision;

// -------------------------------------------------------------------------
// Speed-Accuracy Clock Throttling Modes
// -------------------------------------------------------------------------
typedef enum {
    BIOS_CLOCK_AUTOMATIC    = 0, // Default: Domain-scoped automatic throttling
    BIOS_CLOCK_CONTINUOUS   = 1, // Continuous clock pacing across all code
    BIOS_CLOCK_UNRESTRICTED = 2  // No throttling (maximum host CPU speed)
} BiosClockMode;

// -------------------------------------------------------------------------
// x86 CPU Registers Structure for Interrupt Handling
// -------------------------------------------------------------------------
typedef struct {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
    uint16_t si;
    uint16_t di;
    uint16_t bp;
    uint16_t sp;
    uint16_t flags; // Carry=0x0001, Zero=0x0040, Interrupt=0x0200
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t ss;
} BiosRegs;

// -------------------------------------------------------------------------
// BIOS Data Area (BDA) Structure at Offset 0x00400 (256 bytes)
// -------------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct {
    uint16_t com_port[4];        // 0x400: RS232 I/O port addresses (COM1-COM4)
    uint16_t lpt_port[4];        // 0x408: Parallel printer I/O ports (LPT1-LPT4)
    uint16_t equipment_word;     // 0x410: Installed hardware equipment flags
    uint8_t  post_err_flag;      // 0x412: POST error code flag
    uint16_t memory_size_kb;     // 0x413: Conventional RAM size in KB (640)
    uint8_t  scratch_pad;        // 0x415: Reserved scratchpad
    uint8_t  kbd_flag_1;         // 0x417: Keyboard Shift/Ctrl/Alt status flags
    uint8_t  kbd_flag_2;         // 0x418: Keyboard NumLock/CapsLock/ScrollLock flags
    uint8_t  alt_keypad;         // 0x419: Alt-Keypad entry workspace
    uint16_t kbd_buf_head;       // 0x41A: Keyboard buffer head pointer (offset from 0x400)
    uint16_t kbd_buf_tail;       // 0x41C: Keyboard buffer tail pointer (offset from 0x400)
    uint16_t kbd_buffer[16];     // 0x41E: Keyboard 16-word circular buffer (32 bytes)
    uint8_t  diskette_recal;     // 0x43E: Diskette recalibration status
    uint8_t  diskette_motor;     // 0x43F: Diskette motor status
    uint8_t  diskette_timeout;   // 0x440: Diskette motor timeout counter
    uint8_t  diskette_last_err;  // 0x441: Diskette status return code
    uint8_t  fdc_status[7];      // 0x442: FDC controller status bytes
    uint8_t  video_mode;         // 0x449: Current display mode ID
    uint16_t video_cols;         // 0x44A: Text columns per line (40 or 80)
    uint16_t video_page_size;    // 0x44C: Display page size in bytes
    uint16_t video_page_off;     // 0x44E: Current page start offset in VRAM
    uint16_t cursor_pos[8];      // 0x450: Cursor row/col for 8 display pages
    uint16_t cursor_lines;       // 0x460: Cursor start/end scan lines
    uint8_t  active_page;        // 0x462: Active display page number
    uint16_t crtc_base_port;     // 0x463: CRTC controller base port (0x3D4 or 0x3B4)
    uint8_t  crtc_mode_reg;      // 0x465: Current mode select register value
    uint8_t  crtc_color_palette; // 0x466: Current CGA color palette mask
    uint32_t timer_ticks;        // 0x46C: Daily timer tick counter (18.2 ticks/sec)
    uint8_t  timer_overflow;     // 0x470: Daily timer 24-hour rollover flag
    uint8_t  ctrl_break_flag;    // 0x471: Ctrl-Break flag bit
    uint16_t reset_flag;         // 0x472: Soft reset flag (0x1234 = skip POST)
    uint8_t  fixed_disk_status;  // 0x474: Hard disk status code
    uint8_t  num_fixed_disks;    // 0x475: Number of fixed disk drives connected
    uint8_t  fixed_disk_control; // 0x476: Fixed disk control byte
    uint8_t  fixed_disk_port;    // 0x477: Fixed disk I/O port offset
    uint8_t  print_lpt_timeout[4];// 0x478: LPT printer timeout values
    uint8_t  rs232_com_timeout[4];// 0x47C: COM serial timeout values
    uint16_t kbd_start_off;      // 0x480: Keyboard buffer start offset
    uint16_t kbd_end_off;        // 0x482: Keyboard buffer end offset
    uint8_t  video_rows;         // 0x484: Screen character rows - 1 (EGA/VGA)
    uint16_t char_height;        // 0x485: Character matrix height in scan lines
    uint8_t  video_options;      // 0x487: Video control options flag
    uint8_t  feature_switches;   // 0x488: Feature switches mask
    uint8_t  video_flags_1;      // 0x489: Video display combination flags
    uint8_t  pcjr_video_flags;   // 0x48A: PCjr specific video mode flags
    uint8_t  reserved_bda[117];  // 0x48B-0x4FF: Extended BIOS Data Area workspace
} BiosDataArea;
#pragma pack(pop)

// -------------------------------------------------------------------------
// VRAM Region Address Constants (20-bit Real-Mode Physical Addresses)
//
// These constants define the canonical IBM PC memory-mapped video RAM
// regions used by CGA, MDA, and EGA/VGA display adapters. Any POKE write
// falling within these ranges triggers the VRAM observer callback (if
// registered) to synchronize display output.
// -------------------------------------------------------------------------
#define BIOS_VRAM_EGA_START   0xA0000U  // EGA/VGA graphics framebuffer start
#define BIOS_VRAM_EGA_END     0xAFFFFU  // EGA/VGA graphics framebuffer end
#define BIOS_VRAM_MDA_START   0xB0000U  // MDA monochrome text buffer start
#define BIOS_VRAM_MDA_END     0xB0FFFU  // MDA monochrome text buffer end
#define BIOS_VRAM_CGA_START   0xB8000U  // CGA color text/graphics start
#define BIOS_VRAM_CGA_END     0xBFFFFU  // CGA color text/graphics end

// -------------------------------------------------------------------------
// VRAM Observer Callback Interface
//
// Allows external subsystems (BGI rasterizer, VCon text renderer) to
// receive notification whenever a byte is written to a VRAM region via
// bios_poke(). The observer is called AFTER the byte has been committed
// to the flat 1MB backing store.
//
// Ownership: The BIOS does NOT take ownership of the observer's user_data.
// The caller must ensure the user_data pointer remains valid for the
// lifetime of the observer registration.
// -------------------------------------------------------------------------
typedef void (*BiosVRAMWriteFn)(void* user_data, uint32_t phys_addr, uint8_t value);
typedef uint8_t (*BiosVRAMReadFn)(void* user_data, uint32_t phys_addr);

typedef struct {
    void*            user_data;   // Opaque context passed to callbacks
    BiosVRAMWriteFn  on_write;    // Called on every VRAM byte write
    BiosVRAMReadFn   on_read;     // Called to sync reads (PRINT->VRAM)
} BiosVRAMObserver;

// -------------------------------------------------------------------------
// Memory Provider Hook Interface
// -------------------------------------------------------------------------
typedef uint8_t (*BiosMemReadFn)(void* user_data, uint32_t phys_addr);
typedef void    (*BiosMemWriteFn)(void* user_data, uint32_t phys_addr, uint8_t value);

typedef struct {
    void*           user_data;
    BiosMemReadFn   read_u8;
    BiosMemWriteFn  write_u8;
    uint8_t*        flat_1mb_ptr; // Optional 1MB contiguous block pointer
} BiosMemoryMap;

// -------------------------------------------------------------------------
// Custom Interrupt Handler Function Prototype
// -------------------------------------------------------------------------
typedef struct BiosContext BiosContext;
typedef bool (*BiosIntHandlerFn)(BiosContext* ctx, uint8_t int_num, BiosRegs* regs, void* user_data);

// -------------------------------------------------------------------------
// Public API Functions
// -------------------------------------------------------------------------

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
uint8_t     bios_peek_raw(BiosContext* ctx, uint32_t addr);
void        bios_poke(BiosContext* ctx, uint32_t addr, uint8_t val);
void        bios_poke_raw(BiosContext* ctx, uint32_t addr, uint8_t val);

void        bios_set_vram_observer(BiosContext* ctx, const BiosVRAMObserver* observer);
const BiosVRAMObserver* bios_get_vram_observer(const BiosContext* ctx);

uint8_t     bios_inp(BiosContext* ctx, uint16_t port);
void        bios_out(BiosContext* ctx, uint16_t port, uint8_t val);

void        bios_post_code(BiosContext* ctx, uint8_t code);

bool        bios_register_interrupt(BiosContext* ctx, uint8_t int_num, BiosIntHandlerFn handler, void* user_data);
bool        bios_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_BIOS_BIOS_H
