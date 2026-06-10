/*
 * ---
 * BASIC++ Interpreter - memmap.h
 * ---
 *
 * Pluggable memory map system for PEEK/POKE emulation.
 *
 * PURPOSE:
 * Allows the interpreter to emulate the memory layout of various
 * 8-bit computer platforms. When a map is selected via MEMMAP,
 * the 64K virtual memory segment is pre-filled with correct
 * ROM values, hardware register defaults, screen memory, and
 * I/O register shadows so that PEEK returns the values a real
 * program would expect.
 *
 * SUPPORTED PLATFORMS:
 * MSDOS - MS-DOS (BIOS Data Area, IVT stubs)
 * C64 - Commodore 64 (KERNAL, VIC-II, SID, screen RAM)
 * C128 - Commodore 128
 * VIC20 - VIC-20
 * PLUS4 - Commodore Plus/4
 * PET - Commodore PET 2001/4032/8032
 * ATARI8 - Atari 400/800/XL/XE (ANTIC, POKEY, GTIA)
 * APPLE2 - Apple II / IIe
 * TRS80 - TRS-80 Model I/III
 * SPECTRUM - ZX Spectrum 48K
 * NONE - Bare 64K, all zeros
 *
 * ---
 */

#ifndef BASICPP_MEMMAP_H
#define BASICPP_MEMMAP_H

/* --- Memory Map Type Enum ---
 */
typedef enum MemMapType {
 MMAP_NONE = 0, /* Bare 64K, no presets */
 MMAP_MSDOS, /* MS-DOS (default) */
 MMAP_C64, /* Commodore 64 */
 MMAP_C128, /* Commodore 128 */
 MMAP_VIC20, /* VIC-20 */
 MMAP_PLUS4, /* Commodore Plus/4 */
 MMAP_PET, /* Commodore PET */
 MMAP_ATARI8, /* Atari 400/800/XL/XE */
 MMAP_APPLE2, /* Apple II/IIe */
 MMAP_TRS80, /* TRS-80 Model I/III */
 MMAP_SPECTRUM, /* ZX Spectrum 48K */
 MMAP_COUNT /* sentinel */
} MemMapType;

/*
 * memmap_init - Initialize memory with a platform preset.
 *
 * Clears the entire 64K memory segment, then fills in
 * platform-specific default values (ROM signatures,
 * hardware registers, screen memory, etc.).
 *
 * Parameters:
 * mem - pointer to 64K memory array
 * type - which platform memory map to load
 */
void memmap_init(unsigned char *mem, MemMapType type);

/*
 * memmap_get_name - Return the display name for a map type.
 */
const char *memmap_get_name(MemMapType type);

/*
 * memmap_list - Print all available memory maps to stdout.
 */
void memmap_list(void);

/*
 * memmap_from_string - Parse a platform name string.
 *
 * Returns the MemMapType, or MMAP_COUNT if not recognized.
 * Case-insensitive matching.
 */
MemMapType memmap_from_string(const char *name, int len);

#endif /* BASICPP_MEMMAP_H */
