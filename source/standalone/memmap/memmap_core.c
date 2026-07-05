/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: memmap_core.c
 * Subsystem: Virtual Memory Layout Presets
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Initializes 64K segment presets for C64, Atari, DOS, etc.
 *
 * 2. WHAT TO EXPECT:
 *    Fills virtual RAM with system variables and screen characters.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Initial maps data.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Memory segment mapping bounds.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If RAM fills incorrectly, verify preset startup addresses.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE MEMMAP PRESET REGISTRY CORE
 * File: memmap_core.c
 * ===================================================================== */

#include "memmap_core.h"
#include "../../config.h"
#include <string.h>

static void mem_fill(unsigned char *mem, int start, int len, unsigned char val)
{
    int i;
    if (start < 0 || start + len > MAX_MEM_SEGMENT) return;
    for (i = 0; i < len; i++) {
        mem[start + i] = val;
    }
}

static void mem_set(unsigned char *mem, int addr, unsigned char val)
{
    if (addr >= 0 && addr < MAX_MEM_SEGMENT) {
        mem[addr] = val;
    }
}

static void mem_set16(unsigned char *mem, int addr, int val)
{
    if (addr >= 0 && addr + 1 < MAX_MEM_SEGMENT) {
        mem[addr] = (unsigned char)(val & 0xFF);
        mem[addr + 1] = (unsigned char)((val >> 8) & 0xFF);
    }
}

static void memmap_init_bios_common(unsigned char *mem, unsigned char model_byte)
{
    int i;
    for (i = 0; i < 256; i++) {
        unsigned int offset = 0x1000 + (i * 16);
        unsigned int segment = 0xF000;
        unsigned int addr = i * 4;
        if (addr + 3 < MAX_MEM_SEGMENT) {
            mem[addr] = (unsigned char)(offset & 0xFF);
            mem[addr + 1] = (unsigned char)((offset >> 8) & 0xFF);
            mem[addr + 2] = (unsigned char)(segment & 0xFF);
            mem[addr + 3] = (unsigned char)((segment >> 8) & 0xFF);
        }
    }

    if (0x0400 + 256 <= MAX_MEM_SEGMENT) {
        mem_set16(mem, 0x0413, 0x0280);
        mem_set(mem, 0x0449, 0x03);
        mem_set16(mem, 0x044A, 80);
        mem_set(mem, 0x0462, 0x00);
        mem_set16(mem, 0x0463, 0x03D4);
        mem_set16(mem, 0x0410, 0x0021);
        mem_set16(mem, 0x041A, 0x001E);
        mem_set16(mem, 0x041C, 0x001E);
        mem_set(mem, 0x046C, 0x00);
    }

    if (0xFFFFE < MAX_MEM_SEGMENT) {
        mem[0xFFFFE] = model_byte;
    }
}

static void memmap_init_msdos(unsigned char *mem)
{
    memmap_init_bios_common(mem, 0xFF);
    mem_set16(mem, 0x0413, 0x0280);
    mem_set(mem, 0x0449, 0x03);
    mem_set16(mem, 0x044A, 80);
    mem_set(mem, 0x0462, 0x00);
    mem_set16(mem, 0x0463, 0x03D4);
    mem_set16(mem, 0x0410, 0x0021);
    mem_set16(mem, 0x041A, 0x001E);
    mem_set16(mem, 0x041C, 0x001E);
    mem_set(mem, 0x046C, 0x00);
}

static void memmap_init_ibm_pc(unsigned char *mem)
{
    memmap_init_bios_common(mem, 0xFF);
}

static void memmap_init_ibm_pcjr(unsigned char *mem)
{
    memmap_init_bios_common(mem, 0xFD);
    if (0x0400 + 256 <= MAX_MEM_SEGMENT) {
        mem_set(mem, 0x0449, 0x01);
        mem_set16(mem, 0x044A, 40);
    }
}

static void memmap_init_ibm_xt(unsigned char *mem)
{
    memmap_init_bios_common(mem, 0xFE);
    if (0x0475 < MAX_MEM_SEGMENT) {
        mem[0x0475] = 1;
    }
}

static void memmap_init_ibm_at(unsigned char *mem)
{
    memmap_init_bios_common(mem, 0xFC);
    if (0x0496 < MAX_MEM_SEGMENT) {
        mem[0x0496] = 0x10;
    }
}

static void memmap_init_c64(unsigned char *mem)
{
    mem_set(mem, 0x0000, 0x2F);
    mem_set(mem, 0x0001, 0x37);
    mem_set16(mem, 0x002B, 0x0801);
    mem_set16(mem, 0x002D, 0x0803);
    mem_set16(mem, 0x002F, 0x0803);
    mem_set16(mem, 0x0031, 0x0803);
    mem_set16(mem, 0x0033, 0xA000);
    mem_set16(mem, 0x0037, 0xA000);
    mem_fill(mem, 0x0400, 1000, 0x20);
    mem_fill(mem, 0xD800, 1000, 0x0E);
    mem_set(mem, 0xD011, 0x1B);
    mem_set(mem, 0xD016, 0xC8);
    mem_set(mem, 0xD018, 0x15);
    mem_set(mem, 0xD020, 0x0E);
    mem_set(mem, 0xD021, 0x06);
    mem_set(mem, 0xDC00, 0x7F);
    mem_set(mem, 0xDC01, 0xFF);
    mem_set(mem, 0xDD00, 0x97);

    {
        const char *sig = "CBMBASIC";
        int i;
        for (i = 0; sig[i]; i++) {
            mem_set(mem, 0xA004 + i, (unsigned char)sig[i]);
        }
    }

    mem_set(mem, 0xFF80, 0x03);
    mem_set16(mem, 0xFFFC, 0xFCE2);
    mem_set16(mem, 0xFFFE, 0xFF48);
    mem_set(mem, 0x0286, 0x0E);
    mem_set(mem, 0x00BA, 0x08);
}

static void memmap_init_c128(unsigned char *mem)
{
    memmap_init_c64(mem);
    mem_set(mem, 0xFF00, 0x3E);
    mem_set(mem, 0xFF80, 0x07);
    mem_set16(mem, 0x0033, 0xFF00);
}

static void memmap_init_vic20(unsigned char *mem)
{
    mem_set(mem, 0x0000, 0x2F);
    mem_set(mem, 0x0001, 0x37);
    mem_set16(mem, 0x002B, 0x1001);
    mem_set16(mem, 0x0037, 0x1E00);
    mem_fill(mem, 0x1E00, 506, 0x20);
    mem_fill(mem, 0x9600, 506, 0x01);
    mem_set(mem, 0x9000, 0x0C);
    mem_set(mem, 0x9001, 0x26);
    mem_set(mem, 0x9002, 0x96);
    mem_set(mem, 0x9003, 0x2E);
    mem_set(mem, 0x900E, 0x00);
    mem_set(mem, 0x900F, 0x1B);
    mem_set(mem, 0xFF80, 0x02);
    mem_set16(mem, 0xFFFC, 0xFD22);
}

static void memmap_init_plus4(unsigned char *mem)
{
    mem_set(mem, 0x0000, 0x2F);
    mem_set(mem, 0x0001, 0x37);
    mem_set16(mem, 0x002B, 0x1001);
    mem_set16(mem, 0x0037, 0x8000);
    mem_fill(mem, 0x0C00, 1000, 0x20);
    mem_set(mem, 0xFF06, 0x00);
    mem_set(mem, 0xFF07, 0x00);
    mem_set(mem, 0xFF15, 0x00);
    mem_set(mem, 0xFF16, 0x00);
    mem_set(mem, 0xFF19, 0x00);
    mem_set(mem, 0xFF80, 0x35);
    mem_set16(mem, 0xFFFC, 0xE000);
}

static void memmap_init_pet(unsigned char *mem)
{
    mem_set16(mem, 0x0028, 0x0401);
    mem_set16(mem, 0x0034, 0x8000);
    mem_fill(mem, 0x8000, 1000, 0x20);
    mem_set(mem, 0xE810, 0xFF);
    mem_set(mem, 0xE820, 0xFF);
    mem_set(mem, 0xFF80, 0x04);
    mem_set16(mem, 0xFFFC, 0xFD16);
}

static void memmap_init_atari8(unsigned char *mem)
{
    mem_set(mem, 0x0000, 0x00);
    mem_set16(mem, 0x0058, 0x9C40);
    mem_set16(mem, 0x0080, 0xA000);
    mem_set16(mem, 0x0090, 0xBC1F);
    mem_set(mem, 0x02C4, 0x28);
    mem_set(mem, 0x02C5, 0xCA);
    mem_set(mem, 0x02C6, 0x94);
    mem_set(mem, 0x02C7, 0x46);
    mem_set(mem, 0x02C8, 0x00);
    mem_set(mem, 0xD01A, 0x02);
    mem_set(mem, 0xD20E, 0xFF);
    mem_set(mem, 0xD20F, 0x03);
    mem_set(mem, 0xD300, 0xFF);
    mem_set(mem, 0xD301, 0xFF);
    mem_set(mem, 0xD302, 0x3C);
    mem_set(mem, 0xD303, 0x3C);
    mem_set(mem, 0xD400, 0x22);
    mem_set(mem, 0xD401, 0x02);
    mem_set16(mem, 0xD402, 0x9C20);
    mem_fill(mem, 0x9C40, 960, 0x00);
    mem_set16(mem, 0xFFFC, 0xE477);
    mem_set(mem, 0xFFF7, 0x02);
}

static void memmap_init_apple2(unsigned char *mem)
{
    mem_fill(mem, 0x0400, 0x0400, 0xA0);
    mem_set16(mem, 0x0067, 0x0801);
    mem_set16(mem, 0x0069, 0x0803);
    mem_set16(mem, 0x006B, 0x0803);
    mem_set16(mem, 0x006D, 0x0803);
    mem_set16(mem, 0x0073, 0x9600);
    mem_set16(mem, 0x00AF, 0x9600);
    mem_set(mem, 0xC000, 0x00);
    mem_set(mem, 0xC010, 0x00);
    mem_set(mem, 0xC030, 0x00);
    mem_set(mem, 0xC050, 0x00);
    mem_set(mem, 0xC054, 0x00);
    mem_set(mem, 0xFBB3, 0x06);
    mem_set(mem, 0xFBC0, 0xEA);
    mem_set16(mem, 0xFFFC, 0xFA62);
    mem_set(mem, 0xFF69, 0x4C);
}

static void memmap_init_trs80(unsigned char *mem)
{
    mem_fill(mem, 0x3C00, 1024, 0x20);
    mem_fill(mem, 0x3800, 256, 0x00);
    mem_set16(mem, 0x4000, 0x4200);
    mem_set16(mem, 0x4002, 0x7FFF);
    mem_set(mem, 0x0000, 0xF3);
    mem_set(mem, 0x0001, 0xAF);
    mem_set(mem, 0x0062, 0x52);
}

static void memmap_init_spectrum(unsigned char *mem)
{
    mem_fill(mem, 0x4000, 6144, 0x00);
    mem_fill(mem, 0x5800, 768, 0x38);
    mem_set16(mem, 0x5C53, 0x5CCB);
    mem_set16(mem, 0x5C4B, 0x5CCB);
    mem_set16(mem, 0x5C59, 0x5CCB);
    mem_set16(mem, 0x5C61, 0xFF57);
    mem_set16(mem, 0x5C65, 0xFF58);
    mem_set(mem, 0x5C8D, 0x07);
    mem_set(mem, 0x5C8F, 0x38);
    mem_set(mem, 0x5C48, 0x01);
    mem_set16(mem, 0xFFFC, 0x0000);
    mem_set(mem, 0x0000, 0xF3);
    mem_set(mem, 0x0001, 0xAF);
    mem_set(mem, 0x0002, 0x11);
}

static void memmap_init_ql(unsigned char *mem)
{
    mem_set16(mem, 0x0000, 0x0008);
    mem_set16(mem, 0x0004, 0x0030);
    mem_fill(mem, 0x2000, 0x2000, 0x00);
    mem_set(mem, 0x0100, 0x02);
    mem_set16(mem, 0x0102, 0x0001);
    mem_set16(mem, 0x0104, 128);
    mem_set(mem, 0x0140, 0xFF);
    mem_fill(mem, 0x0160, 32, 0x00);
    mem_set(mem, 0x0110, 0x00);
}

static const struct {
    const char *name;
    MemMapType type;
} memmap_core_table[] = {
    { "NONE", MMAP_NONE },
    { "MSDOS", MMAP_MSDOS },
    { "IBMPC", MMAP_IBM_PC },
    { "PCJR", MMAP_IBM_PCJR },
    { "PCXT", MMAP_IBM_XT },
    { "PCAT", MMAP_IBM_AT },
    { "C64", MMAP_C64 },
    { "C128", MMAP_C128 },
    { "VIC20", MMAP_VIC20 },
    { "PLUS4", MMAP_PLUS4 },
    { "PET", MMAP_PET },
    { "ATARI8", MMAP_ATARI8 },
    { "APPLE2", MMAP_APPLE2 },
    { "TRS80", MMAP_TRS80 },
    { "SPECTRUM", MMAP_SPECTRUM },
    { "QL", MMAP_QL },
    { NULL, MMAP_COUNT }
};

void memmap_core_init(unsigned char *mem, MemMapType type)
{
    memset(mem, 0, (size_t)MAX_MEM_SEGMENT);

    switch (type) {
    case MMAP_MSDOS: memmap_init_msdos(mem); break;
    case MMAP_IBM_PC: memmap_init_ibm_pc(mem); break;
    case MMAP_IBM_PCJR: memmap_init_ibm_pcjr(mem); break;
    case MMAP_IBM_XT: memmap_init_ibm_xt(mem); break;
    case MMAP_IBM_AT: memmap_init_ibm_at(mem); break;
    case MMAP_C64: memmap_init_c64(mem); break;
    case MMAP_C128: memmap_init_c128(mem); break;
    case MMAP_VIC20: memmap_init_vic20(mem); break;
    case MMAP_PLUS4: memmap_init_plus4(mem); break;
    case MMAP_PET: memmap_init_pet(mem); break;
    case MMAP_ATARI8: memmap_init_atari8(mem); break;
    case MMAP_APPLE2: memmap_init_apple2(mem); break;
    case MMAP_TRS80: memmap_init_trs80(mem); break;
    case MMAP_SPECTRUM: memmap_init_spectrum(mem); break;
    case MMAP_QL: memmap_init_ql(mem); break;
    case MMAP_NONE:
    default:
        break;
    }
}

const char *memmap_core_get_name(MemMapType type)
{
    int i;
    for (i = 0; memmap_core_table[i].name != NULL; i++) {
        if (memmap_core_table[i].type == type) {
            return memmap_core_table[i].name;
        }
    }
    return "UNKNOWN";
}

MemMapType memmap_core_from_string(const char *name, int len)
{
    int i;
    if (len == 2) {
        char a = name[0];
        char b = name[1];
        if (a >= 'a' && a <= 'z') a = (char)(a - 32);
        if (b >= 'a' && b <= 'z') b = (char)(b - 32);
        if (a == 'X' && b == 'T') return MMAP_IBM_XT;
        if (a == 'A' && b == 'T') return MMAP_IBM_AT;
        if (a == 'P' && b == 'C') return MMAP_IBM_PC;
    }

    for (i = 0; memmap_core_table[i].name != NULL; i++) {
        const char *n = memmap_core_table[i].name;
        int nlen = (int)strlen(n);
        int j, match;

        if (nlen != len) continue;

        match = 1;
        for (j = 0; j < len; j++) {
            char a = name[j];
            char b = n[j];
            if (a >= 'a' && a <= 'z') a = (char)(a - 32);
            if (b >= 'a' && b <= 'z') b = (char)(b - 32);
            if (a != b) { match = 0; break; }
        }
        if (match) {
            return memmap_core_table[i].type;
        }
    }
    return MMAP_COUNT;
}

MemMapType memmap_core_default_for_dialect(int dialect_id)
{
    switch (dialect_id) {
    case 0: // 6 / default
        return MMAP_NONE;
    default:
        return MMAP_MSDOS;
    }
}
