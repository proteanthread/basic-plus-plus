/*
 * =====================================================================
 * BASIC++ Interpreter - memmap.c
 * =====================================================================
 *
 * Pluggable memory map system implementation.
 *
 * Each platform preset fills the 64K virtual memory segment with
 * the default values a real machine would have at cold boot.
 * This allows PEEK to return correct hardware register values,
 * ROM signatures, and screen memory contents.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include <stdio.h>
#include <string.h>
#include "memmap.h"
#include "config.h"

/* =====================================================================
 * Helper: fill a range of memory with a value
 * =====================================================================
 */
static void mem_fill(unsigned char *mem, int start, int len,
                     unsigned char val)
{
    int i;
    if (start < 0 || start + len > MAX_MEM_SEGMENT) return;
    for (i = 0; i < len; i++)
        mem[start + i] = val;
}

/* Helper: set a single byte */
static void mem_set(unsigned char *mem, int addr,
                    unsigned char val)
{
    if (addr >= 0 && addr < MAX_MEM_SEGMENT)
        mem[addr] = val;
}

/* Helper: set a 16-bit little-endian word */
static void mem_set16(unsigned char *mem, int addr,
                      int val)
{
    if (addr >= 0 && addr + 1 < MAX_MEM_SEGMENT) {
        mem[addr]     = (unsigned char)(val & 0xFF);
        mem[addr + 1] = (unsigned char)((val >> 8) & 0xFF);
    }
}

/* =====================================================================
 * MS-DOS Memory Map
 * =====================================================================
 * 0x0000-0x03FF: Interrupt Vector Table (256 dword entries)
 * 0x0400-0x04FF: BIOS Data Area (BDA)
 * 0x0413-0x0414: Conventional memory size (640K = 0x0280)
 * 0x0449:        Current video mode (3 = 80x25 color text)
 * 0x044A-0x044B: Columns per line (80)
 * 0x0462:        Active video page (0)
 * 0x0463-0x0464: CRT controller base port (0x03D4)
 */
static void memmap_init_msdos(unsigned char *mem)
{
    /* BIOS Data Area */
    mem_set16(mem, 0x0413, 0x0280);  /* 640K */
    mem_set(mem, 0x0449, 0x03);      /* Video mode 3 */
    mem_set16(mem, 0x044A, 80);      /* 80 columns */
    mem_set(mem, 0x0462, 0x00);      /* Page 0 */
    mem_set16(mem, 0x0463, 0x03D4);  /* CRT port */

    /* Equipment word: 80-col, 1 floppy, VGA */
    mem_set16(mem, 0x0410, 0x0021);

    /* Keyboard buffer head/tail */
    mem_set16(mem, 0x041A, 0x001E);
    mem_set16(mem, 0x041C, 0x001E);

    /* Timer tick count (approx 0 at boot) */
    mem_set(mem, 0x046C, 0x00);
}

/* =====================================================================
 * Commodore 64 Memory Map
 * =====================================================================
 * $0000: CPU data direction register
 * $0001: CPU port (bank switching): $37 = BASIC+KERNAL+I/O visible
 * $002B-$002C: Start of BASIC text ($0801)
 * $0037-$0038: Top of BASIC text
 * $0033-$0034: Start of BASIC variables
 * $0400-$07E7: Screen memory (1000 bytes, default spaces $20)
 * $0801: Typical BASIC start
 * $A000-$BFFF: BASIC ROM (8K)
 * $C000-$CFFF: Upper RAM
 * $D000-$D3FF: VIC-II registers
 * $D400-$D7FF: SID registers
 * $D800-$DBE7: Color RAM (1000 nybbles)
 * $DC00-$DCFF: CIA 1
 * $DD00-$DDFF: CIA 2
 * $E000-$FFFF: KERNAL ROM (8K)
 */
static void memmap_init_c64(unsigned char *mem)
{
    /* CPU port */
    mem_set(mem, 0x0000, 0x2F);  /* DDR */
    mem_set(mem, 0x0001, 0x37);  /* BASIC+KERNAL+I/O */

    /* BASIC pointers */
    mem_set16(mem, 0x002B, 0x0801);  /* Start of BASIC */
    mem_set16(mem, 0x002D, 0x0803);  /* Start of variables */
    mem_set16(mem, 0x002F, 0x0803);  /* Start of arrays */
    mem_set16(mem, 0x0031, 0x0803);  /* End of arrays */
    mem_set16(mem, 0x0033, 0xA000);  /* Bottom of strings */
    mem_set16(mem, 0x0037, 0xA000);  /* Top of BASIC memory */

    /* Screen memory: 1000 spaces */
    mem_fill(mem, 0x0400, 1000, 0x20);

    /* Color RAM: light blue (14) */
    mem_fill(mem, 0xD800, 1000, 0x0E);

    /* VIC-II registers */
    mem_set(mem, 0xD011, 0x1B);  /* Screen control #1 */
    mem_set(mem, 0xD016, 0xC8);  /* Screen control #2 */
    mem_set(mem, 0xD018, 0x15);  /* Memory setup */
    mem_set(mem, 0xD020, 0x0E);  /* Border: light blue */
    mem_set(mem, 0xD021, 0x06);  /* Background: blue */

    /* SID: all zeros at boot (silence) */

    /* CIA 1 */
    mem_set(mem, 0xDC00, 0x7F);  /* Port A */
    mem_set(mem, 0xDC01, 0xFF);  /* Port B */

    /* CIA 2 */
    mem_set(mem, 0xDD00, 0x97);  /* Port A (VIC bank) */

    /* BASIC ROM signature at $A004 "CBMBASIC" */
    {
        const char *sig = "CBMBASIC";
        int i;
        for (i = 0; sig[i]; i++)
            mem_set(mem, 0xA004 + i, (unsigned char)sig[i]);
    }

    /* KERNAL version byte */
    mem_set(mem, 0xFF80, 0x03);  /* KERNAL rev 3 */

    /* Hardware vectors */
    mem_set16(mem, 0xFFFC, 0xFCE2);  /* RESET vector */
    mem_set16(mem, 0xFFFE, 0xFF48);  /* IRQ vector */

    /* 646: Current text color */
    mem_set(mem, 0x0286, 0x0E);  /* Light blue */

    /* $BA: Current device number */
    mem_set(mem, 0x00BA, 0x08);  /* Device 8 (disk) */
}

/* =====================================================================
 * Commodore 128 Memory Map
 * =====================================================================
 * Similar to C64 but with extended features.
 * Bank 0 is C64-compatible, we preset the key differences.
 */
static void memmap_init_c128(unsigned char *mem)
{
    /* Start with C64 base */
    memmap_init_c64(mem);

    /* C128-specific: MMU configuration register */
    mem_set(mem, 0xFF00, 0x3E);  /* MMU config */

    /* C128 BASIC 7.0 version ID */
    mem_set(mem, 0xFF80, 0x07);  /* Version 7 */

    /* 128K RAM indicator */
    mem_set16(mem, 0x0033, 0xFF00);  /* More string space */
}

/* =====================================================================
 * VIC-20 Memory Map
 * =====================================================================
 * $0000-$03FF: Zero page, stack, system
 * $1000-$1DFF: Screen memory (default)
 * $1E00-$1FFF: Color memory
 * $9000-$900F: VIC chip registers
 * $9110-$911F: VIA #1
 * $9120-$912F: VIA #2
 * $C000-$DFFF: BASIC ROM (8K)
 * $E000-$FFFF: KERNAL ROM (8K)
 *
 * Unexpanded VIC-20 has 5K RAM: $0000-$03FF + $1000-$1FFF
 */
static void memmap_init_vic20(unsigned char *mem)
{
    /* CPU port */
    mem_set(mem, 0x0000, 0x2F);
    mem_set(mem, 0x0001, 0x37);

    /* BASIC start pointer */
    mem_set16(mem, 0x002B, 0x1001);  /* BASIC starts at $1001 */
    mem_set16(mem, 0x0037, 0x1E00);  /* Top of BASIC memory */

    /* Screen memory: 22x23 = 506 chars */
    mem_fill(mem, 0x1E00, 506, 0x20);

    /* Color memory */
    mem_fill(mem, 0x9600, 506, 0x01);  /* White */

    /* VIC chip registers */
    mem_set(mem, 0x9000, 0x0C);  /* Horiz pos */
    mem_set(mem, 0x9001, 0x26);  /* Vert pos */
    mem_set(mem, 0x9002, 0x96);  /* # columns */
    mem_set(mem, 0x9003, 0x2E);  /* # rows */
    mem_set(mem, 0x900E, 0x00);  /* Aux color */
    mem_set(mem, 0x900F, 0x1B);  /* Border+bg */

    /* KERNAL version */
    mem_set(mem, 0xFF80, 0x02);

    /* Reset vector */
    mem_set16(mem, 0xFFFC, 0xFD22);
}

/* =====================================================================
 * Commodore Plus/4 Memory Map
 * =====================================================================
 * $0000-$0001: CPU port
 * $0400-$07FF: Screen memory (default location)
 * $FF00-$FF1F: TED registers
 * $8000-$BFFF: BASIC 3.5 ROM
 * $C000-$FFFF: KERNAL ROM
 */
static void memmap_init_plus4(unsigned char *mem)
{
    mem_set(mem, 0x0000, 0x2F);
    mem_set(mem, 0x0001, 0x37);

    /* BASIC pointers */
    mem_set16(mem, 0x002B, 0x1001);
    mem_set16(mem, 0x0037, 0x8000);

    /* Screen memory */
    mem_fill(mem, 0x0C00, 1000, 0x20);

    /* TED registers */
    mem_set(mem, 0xFF06, 0x00);  /* Horizontal pos */
    mem_set(mem, 0xFF07, 0x00);  /* Vertical pos */
    mem_set(mem, 0xFF15, 0x00);  /* Background 0 */
    mem_set(mem, 0xFF16, 0x00);  /* Background 1 */
    mem_set(mem, 0xFF19, 0x00);  /* Border */

    /* BASIC 3.5 signature */
    mem_set(mem, 0xFF80, 0x35);

    mem_set16(mem, 0xFFFC, 0xE000);
}

/* =====================================================================
 * Commodore PET Memory Map
 * =====================================================================
 * $0000-$0400: Zero page, stack, system area
 * $8000-$83E7: Screen memory (40x25 = 1000 bytes)
 * $B000-$DFFF: BASIC ROM (12K, BASIC 4.0)
 * $E000-$E7FF: Editor ROM (2K)
 * $F000-$FFFF: KERNAL ROM (4K)
 */
static void memmap_init_pet(unsigned char *mem)
{
    /* BASIC pointers */
    mem_set16(mem, 0x0028, 0x0401);  /* Start of BASIC */
    mem_set16(mem, 0x0034, 0x8000);  /* Top of memory */

    /* Screen memory: 40x25 = 1000 spaces */
    mem_fill(mem, 0x8000, 1000, 0x20);

    /* PIA registers */
    mem_set(mem, 0xE810, 0xFF);  /* PIA 1 */
    mem_set(mem, 0xE820, 0xFF);  /* PIA 2 */

    /* BASIC 4.0 marker */
    mem_set(mem, 0xFF80, 0x04);

    /* Reset vector */
    mem_set16(mem, 0xFFFC, 0xFD16);
}

/* =====================================================================
 * Atari 8-bit Memory Map
 * =====================================================================
 * $0000-$00FF: Zero page (OS shadow registers)
 * $0040-$00FF: BASIC zero page
 * $0200-$02FF: Handler table, IOCB area
 * $D000-$D01F: GTIA (graphics)
 * $D200-$D20F: POKEY (audio, keyboard, serial, timers)
 * $D300-$D303: PIA (I/O ports)
 * $D400-$D40F: ANTIC (display list processor)
 * $E000-$FFFF: OS ROM (8K)
 * $A000-$BFFF: BASIC ROM (8K, cartridge)
 *
 * Color registers shadow at zero page ($C4-$C8)
 */
static void memmap_init_atari8(unsigned char *mem)
{
    /* Zero page OS variables */
    mem_set(mem, 0x0000, 0x00);  /* LINZBS */
    mem_set16(mem, 0x0058, 0x9C40);  /* SAVMSC screen addr */

    /* BASIC pointers */
    mem_set16(mem, 0x0080, 0xA000);  /* LOMEM */
    mem_set16(mem, 0x0090, 0xBC1F);  /* MEMTOP */

    /* Color register shadows */
    mem_set(mem, 0x02C4, 0x28);  /* COLPF0 */
    mem_set(mem, 0x02C5, 0xCA);  /* COLPF1 */
    mem_set(mem, 0x02C6, 0x94);  /* COLPF2 */
    mem_set(mem, 0x02C7, 0x46);  /* COLPF3 */
    mem_set(mem, 0x02C8, 0x00);  /* COLBK (background) */

    /* GTIA registers */
    mem_set(mem, 0xD01A, 0x02);  /* PRIOR */

    /* POKEY registers */
    mem_set(mem, 0xD20E, 0xFF);  /* IRQEN - IRQ enable */
    mem_set(mem, 0xD20F, 0x03);  /* SKSTAT */

    /* PIA */
    mem_set(mem, 0xD300, 0xFF);  /* PORTA (joystick) */
    mem_set(mem, 0xD301, 0xFF);  /* PORTB (bank switch) */
    mem_set(mem, 0xD302, 0x3C);  /* PACTL */
    mem_set(mem, 0xD303, 0x3C);  /* PBCTL */

    /* ANTIC */
    mem_set(mem, 0xD400, 0x22);  /* DMACTL */
    mem_set(mem, 0xD401, 0x02);  /* CHACTL */
    mem_set16(mem, 0xD402, 0x9C20);  /* DLISTL/H */

    /* Screen memory (40x24 = 960 bytes at default) */
    mem_fill(mem, 0x9C40, 960, 0x00);  /* Spaces in ATASCII */

    /* Reset vector */
    mem_set16(mem, 0xFFFC, 0xE477);

    /* OS version (XL/XE) */
    mem_set(mem, 0xFFF7, 0x02);
}

/* =====================================================================
 * Apple II Memory Map
 * =====================================================================
 * $0000-$00FF: Zero page
 * $0100-$01FF: Stack
 * $0200-$02FF: Input buffer
 * $0300-$03FF: DOS vectors
 * $0400-$07FF: Text page 1 (40x24 = 960)
 * $0800-$0BFF: Text page 2
 * $2000-$3FFF: HGR page 1
 * $4000-$5FFF: HGR page 2
 * $C000-$C0FF: Soft switches (I/O)
 * $D000-$F7FF: Applesoft BASIC ROM (10K)
 * $F800-$FFFF: Monitor ROM (2K)
 */
static void memmap_init_apple2(unsigned char *mem)
{
    /* Text page 1: 40x24 spaces (high bit set = normal) */
    mem_fill(mem, 0x0400, 0x0400, 0xA0);  /* Space with high bit */

    /* Zero page BASIC pointers */
    mem_set16(mem, 0x0067, 0x0801);  /* TXTTAB: start of program */
    mem_set16(mem, 0x0069, 0x0803);  /* VARTAB: start of variables */
    mem_set16(mem, 0x006B, 0x0803);  /* ARYTAB */
    mem_set16(mem, 0x006D, 0x0803);  /* STREND */
    mem_set16(mem, 0x0073, 0x9600);  /* HIMEM */
    mem_set16(mem, 0x00AF, 0x9600);  /* MEMSIZ */

    /* Soft switches: default states */
    mem_set(mem, 0xC000, 0x00);  /* Keyboard strobe */
    mem_set(mem, 0xC010, 0x00);  /* Clear keyboard */
    mem_set(mem, 0xC030, 0x00);  /* Speaker toggle */
    mem_set(mem, 0xC050, 0x00);  /* Text mode */
    mem_set(mem, 0xC054, 0x00);  /* Page 1 */

    /* ROM identification */
    mem_set(mem, 0xFBB3, 0x06);  /* Apple IIe identifier */
    mem_set(mem, 0xFBC0, 0xEA);  /* Apple IIe secondary */

    /* Reset vector */
    mem_set16(mem, 0xFFFC, 0xFA62);

    /* Monitor entry */
    mem_set(mem, 0xFF69, 0x4C);  /* JMP for monitor */
}

/* =====================================================================
 * TRS-80 Memory Map
 * =====================================================================
 * $0000-$2FFF: ROM (12K Level II BASIC)
 * $3000-$37FF: Unused / ROM mirror
 * $3800-$38FF: Keyboard memory map
 * $3C00-$3FFF: Video RAM (64x16 = 1024)
 * $4000-$41FF: System area
 * $4200-$FFFF: User RAM (to top of installed memory)
 *
 * 64-column display, position = row * 64 + col
 */
static void memmap_init_trs80(unsigned char *mem)
{
    /* Video RAM: 64x16 = 1024 spaces */
    mem_fill(mem, 0x3C00, 1024, 0x20);

    /* Keyboard map: all keys up */
    mem_fill(mem, 0x3800, 256, 0x00);

    /* System variables */
    mem_set16(mem, 0x4000, 0x4200);  /* Start of BASIC */
    mem_set16(mem, 0x4002, 0x7FFF);  /* Top of memory (32K) */

    /* ROM identification bytes */
    mem_set(mem, 0x0000, 0xF3);  /* DI instruction */
    mem_set(mem, 0x0001, 0xAF);  /* XOR A */

    /* Level II BASIC signature area */
    mem_set(mem, 0x0062, 0x52);  /* 'R' for Radio Shack */
}

/* =====================================================================
 * ZX Spectrum 48K Memory Map
 * =====================================================================
 * $0000-$3FFF: ROM (16K - Spectrum BASIC + system)
 * $4000-$57FF: Screen bitmap (6144 bytes)
 * $5800-$5AFF: Attribute memory (768 bytes, 32x24)
 * $5B00-$5CBF: Printer buffer / system vars
 * $5CB6-$5CCA: System variables area
 * $5CCB-$FFFF: User RAM (BASIC, variables, stack)
 *
 * Screen: 256x192 pixels, 8x8 char cells, attributes per cell
 * Attribute format: FBPPPIII (Flash, Bright, Paper 0-7, Ink 0-7)
 */
static void memmap_init_spectrum(unsigned char *mem)
{
    /* Screen memory: clear (all black) */
    mem_fill(mem, 0x4000, 6144, 0x00);

    /* Attribute memory: white ink on black paper, no flash */
    /* Default: 0x38 = 00 111 000 = bright off, paper 7, ink 0 */
    /* Actually default is: 0x38 = paper white, ink black */
    mem_fill(mem, 0x5800, 768, 0x38);

    /* System variables */
    mem_set16(mem, 0x5C53, 0x5CCB);  /* PROG: start of BASIC */
    mem_set16(mem, 0x5C4B, 0x5CCB);  /* VARS: start of vars */
    mem_set16(mem, 0x5C59, 0x5CCB);  /* E_LINE */
    mem_set16(mem, 0x5C61, 0xFF57);  /* STKEND */
    mem_set16(mem, 0x5C65, 0xFF58);  /* STKBOT */
    mem_set(mem, 0x5C8D, 0x07);     /* ATTR_P: current attr */
    mem_set(mem, 0x5C8F, 0x38);     /* MASK_P */
    mem_set(mem, 0x5C48, 0x01);     /* BORDCR: border color */

    /* ROM: NMI/IRQ vectors */
    mem_set16(mem, 0xFFFC, 0x0000);  /* Reset vector */

    /* ROM identification */
    mem_set(mem, 0x0000, 0xF3);  /* DI */
    mem_set(mem, 0x0001, 0xAF);  /* XOR A */
    mem_set(mem, 0x0002, 0x11);  /* LD DE,... */
}

/* =====================================================================
 * Name Table
 * =====================================================================
 */
static const struct {
    const char *name;
    const char *desc;
    MemMapType type;
} memmap_table[] = {
    { "NONE",     "Bare 64K, no presets",              MMAP_NONE },
    { "MSDOS",    "MS-DOS (BIOS Data Area, 640K)",     MMAP_MSDOS },
    { "C64",      "Commodore 64 (VIC-II, SID, CIA)",   MMAP_C64 },
    { "C128",     "Commodore 128 (C64 + MMU)",         MMAP_C128 },
    { "VIC20",    "Commodore VIC-20",                  MMAP_VIC20 },
    { "PLUS4",    "Commodore Plus/4 (TED)",            MMAP_PLUS4 },
    { "PET",      "Commodore PET 2001/4032",           MMAP_PET },
    { "ATARI8",   "Atari 400/800/XL/XE",              MMAP_ATARI8 },
    { "APPLE2",   "Apple II / IIe",                    MMAP_APPLE2 },
    { "TRS80",    "TRS-80 Model I/III",                MMAP_TRS80 },
    { "SPECTRUM", "ZX Spectrum 48K",                   MMAP_SPECTRUM },
    { NULL, NULL, MMAP_COUNT }
};

/* =====================================================================
 * Public API
 * =====================================================================
 */

void memmap_init(unsigned char *mem, MemMapType type)
{
    /* Clear entire 64K */
    memset(mem, 0, (size_t)MAX_MEM_SEGMENT);

    switch (type) {
        case MMAP_MSDOS:    memmap_init_msdos(mem);    break;
        case MMAP_C64:      memmap_init_c64(mem);      break;
        case MMAP_C128:     memmap_init_c128(mem);     break;
        case MMAP_VIC20:    memmap_init_vic20(mem);    break;
        case MMAP_PLUS4:    memmap_init_plus4(mem);    break;
        case MMAP_PET:      memmap_init_pet(mem);      break;
        case MMAP_ATARI8:   memmap_init_atari8(mem);   break;
        case MMAP_APPLE2:   memmap_init_apple2(mem);   break;
        case MMAP_TRS80:    memmap_init_trs80(mem);    break;
        case MMAP_SPECTRUM: memmap_init_spectrum(mem);  break;
        case MMAP_NONE:
        default:
            break;  /* Already zeroed */
    }
}

const char *memmap_get_name(MemMapType type)
{
    int i;
    for (i = 0; memmap_table[i].name != NULL; i++) {
        if (memmap_table[i].type == type)
            return memmap_table[i].name;
    }
    return "UNKNOWN";
}

void memmap_list(void)
{
    int i;
    printf("Available memory maps:\n");
    for (i = 0; memmap_table[i].name != NULL; i++) {
        printf("  %-10s %s\n",
               memmap_table[i].name,
               memmap_table[i].desc);
    }
}

MemMapType memmap_from_string(const char *name, int len)
{
    int i;
    for (i = 0; memmap_table[i].name != NULL; i++) {
        const char *n = memmap_table[i].name;
        int nlen = (int)strlen(n);
        int j, match;

        if (nlen != len) continue;

        match = 1;
        for (j = 0; j < len; j++) {
            char a = name[j];
            char b = n[j];
            /* Case-insensitive compare */
            if (a >= 'a' && a <= 'z') a = (char)(a - 32);
            if (b >= 'a' && b <= 'z') b = (char)(b - 32);
            if (a != b) { match = 0; break; }
        }
        if (match)
            return memmap_table[i].type;
    }
    return MMAP_COUNT;  /* Not found */
}
