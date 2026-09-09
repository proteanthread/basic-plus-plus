// FILENAME: qheader.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (qheader.c, stmt_qsave.c, stmt_qload.c, stmt_qrun.c, bload.c, bsave.c, brun.c)
// NEEDS: standard C17 (<stdint.h>, <stdbool.h>, <stddef.h>)
// Declares the 256-byte modern binary package header, paired UDB header, and vintage 7-byte header.

#ifndef STATEMENTS_FILESYSTEM_BINARY_OPS_QHEADER_H
#define STATEMENTS_FILESYSTEM_BINARY_OPS_QHEADER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define QHEADER_MAGIC_PRIMARY     0xFD
#define QHEADER_MAGIC_EXTENDED    0xFE
#define QHEADER_FORMAT_ID         "QB256\0"
#define QHEADER_VERSION_CURRENT   1

#define QPAYLOAD_MEMORY_IMAGE     0x01
#define QPAYLOAD_FRAMEBUFFER      0x02
#define QPAYLOAD_ARRAY_MATRIX     0x03
#define QPAYLOAD_BYTECODE_PCODE   0x04
#define QPAYLOAD_NATIVE_PLUGIN    0x05
#define QPAYLOAD_RESOURCE_BLOB    0x06
#define QPAYLOAD_VIRTUAL_LIBRARY  0x07
#define QPAYLOAD_VIRTUAL_PLUGIN   0x08
#define QPAYLOAD_VIRTUAL_DEVICE   0x09
#define QPAYLOAD_IOT_FIRMWARE     0x0A // Standalone IoT microcontroller firmware
#define QPAYLOAD_OPTION_ROM_8086  0x0B // Raw 8086 Option ROM / BIOS extension for libcpu8086
#define QPAYLOAD_RAMBANK_IMAGE    0x0C // 1MB Segmented RAMBank snapshot (banks 1..64)
#define QPAYLOAD_UDB_MAPPING      0x0D // User Defined Block hardware/memory map

#define QFLAG_COMPRESSED          (1U << 0)
#define QFLAG_READONLY            (1U << 1)
#define QFLAG_POSITION_INDEP      (1U << 2)
#define QFLAG_AUTORUN_BOOT        (1U << 3) // Autonomous IoT power-on execution without host
#define QFLAG_REQUIRES_GPIO       (1U << 4) // Microcontroller GPIO peripheral access required
#define QFLAG_HAS_UDB_MAP         (1U << 5) // User Defined Block (UDB) mapping active
#define QFLAG_HAS_UDD_MAP         (1U << 6) // User Defined Device (UDD) mapping active
#define QFLAG_HAS_PAIRED_HEADER   (1U << 7) // Followed immediately by 256-byte QUdbHeader256

#define QARCH_UNIVERSAL_VM        0x0000 // Universal Bytecode / Portable Data
#define QARCH_X86_REAL_8086       0x0001 // x86 16-bit Real Mode (8086/8088)
#define QARCH_M6502               0x0002 // MOS Technology 6502
#define QARCH_M6809               0x0003 // Motorola 6809
#define QARCH_Z80                 0x0004 // Zilog Z80
#define QARCH_X86_32              0x0005 // x86 32-bit Protected Mode
#define QARCH_X86_64              0x0006 // x86_64
#define QARCH_IOT_GENERIC         0x0010 // Generic IoT / Embedded Microcontroller
#define QARCH_ESP32_XTENSA        0x0011 // Espressif ESP32 (Xtensa LX6/LX7)
#define QARCH_IOT_RISCV           0x0012 // RISC-V Microcontroller
#define QARCH_ARM_CORTEX_M        0x0013 // ARM Cortex-M Series

#define QSUBSYS_IBM_PC_BIOS       0x0001 // IBM PC/XT/AT BIOS
#define QSUBSYS_FREESTANDING_IOT  0x0010 // Freestanding IoT HAL
// Note: Subsystem slots 0x0002 through 0x000F are reserved open slots for future
// machine BIOS implementations (C64 KERNAL, Atari OS, Apple SOS, CoCo, etc.).

// User Defined Block (UDB) Subtype Definitions
// Note: UDB stands strictly for User Defined Block (memory, cpu context, registers, IoT state)
// and NOT User Defined Bus (bus virtualization is planned for future releases).
#define QUDB_BLOCK_GENERIC        0x00 // Generic User Defined Block
#define QUDB_BLOCK_MEMORY         0x01 // Memory Window / Segment / RAMBank slice
#define QUDB_BLOCK_CPU_CONTEXT    0x02 // CPU Register Context & Vector Table
#define QUDB_BLOCK_IO_REGISTERS   0x03 // Memory-Mapped I/O & Register Window
#define QUDB_BLOCK_ROM_WINDOW     0x04 // BIOS Option ROM / Firmware Window
#define QUDB_BLOCK_RAMBANK        0x05 // RAMBank Matrix Mapping (banks 1..64)
#define QUDB_BLOCK_IOT_CALIBRAT   0x06 // IoT Peripheral Pinout & Sensor Calibration
#define QUDB_BLOCK_IOT_STATE      0x07 // IoT Sleep / Deep-Sleep RAM Snapshot
#define QUDB_BLOCK_UDT_SCHEMA     0x08 // User Defined Type Struct Schema
#define QUDB_BLOCK_CUSTOM_EXT     0x09 // Custom Extension & Plugin Descriptor

// Backward-compatibility aliases for legacy QUD_TYPE_* references
#define QUD_TYPE_GENERIC          QUDB_BLOCK_GENERIC
#define QUD_TYPE_CPU_CONTEXT      QUDB_BLOCK_CPU_CONTEXT
#define QUD_TYPE_BIOS_ROM_MAP     QUDB_BLOCK_ROM_WINDOW
#define QUD_TYPE_UDD_DEVICE       QUDB_BLOCK_IO_REGISTERS
#define QUD_TYPE_IOT_PROFILE      QUDB_BLOCK_IOT_STATE
#define QUD_TYPE_RAMBANK_MAP      QUDB_BLOCK_RAMBANK
#define QUD_TYPE_UDT_SCHEMA       QUDB_BLOCK_UDT_SCHEMA
#define QUD_TYPE_CUSTOM_EXT       QUDB_BLOCK_CUSTOM_EXT

#define QSYM_TYPE_STATEMENT       0x01
#define QSYM_TYPE_FUNCTION        0x02
#define QSYM_TYPE_DEVICE          0x03
#define QSYM_TYPE_VARIABLE        0x04

#pragma pack(push, 1)

// Vintage 7-byte header used by GW-BASIC, BASICA, and retro emulators
typedef struct {
    uint8_t  magic;       // 0xFD
    uint16_t segment;     // Segment address (little-endian)
    uint16_t offset;      // Offset address (little-endian)
    uint16_t length;      // Block byte count (little-endian)
} BSaveHeader;

// 32-byte Export Symbol Descriptor for dynamic plugins and libraries
typedef struct {
    char     name[20];    // Symbol name (null-terminated ASCII)
    uint8_t  sym_type;    // QSYM_TYPE_*
    uint8_t  param_count; // Number of expected parameters (or 0xFF for variadic)
    uint16_t reserved;    // Alignment padding
    uint64_t entry_addr;  // Memory offset or bytecode instruction pointer
} QExportSymbol;

// Sector-aligned 256-byte primary header for modern QSAVE, QLOAD, QRUN packages
typedef struct {
    uint8_t  magic_primary;          // 0xFD (Vintage memory image indicator)
    uint8_t  magic_extended;         // 0xFE (Modern extended indicator)
    char     format_id[6];           // "QB256\0" ASCII string
    uint8_t  header_version;         // Format version (1)
    uint8_t  arch_bits;              // 16, 32, or 64 bitness
    uint8_t  endianness;             // 0 = Little-endian, 1 = Big-endian
    uint8_t  payload_type;           // QPAYLOAD_*
    uint16_t target_arch;            // QARCH_* (16-bit CPU Architecture ID)
    uint16_t target_subsystem;       // QSUBSYS_* (16-bit Machine/BIOS ID)
    uint64_t segment_address;        // 64-bit Segment or Virtual Context ID / RAMBank
    uint64_t offset_address;         // 64-bit Memory Offset / Base Address
    uint64_t block_length;          // 64-bit Payload Size in bytes
    uint64_t entry_point;            // 64-bit Execution Entry Point for QRUN
    uint64_t uncompressed_length;    // 64-bit Original Size before compression
    uint32_t crc32;                  // 32-bit CRC-32 Checksum of Payload
    uint32_t flags;                  // QFLAG_* feature flags
    uint16_t chain_count;            // Number of chained 256-byte descriptor headers
    uint16_t paired_header_type;     // Type of paired header (QUDB_BLOCK_*)
    uint32_t export_count;           // Number of exported symbols
    uint64_t export_table_offset;    // Offset to export symbol table
    uint8_t  reserved_meta[16];      // Multi-socket, UDB, and NUMA memory attributes
    char     module_name[32];        // Module description (null-terminated)
    char     compiler_sig[32];       // Compiler signature (null-terminated)
    uint8_t  reserved[96];           // Reserved padding to exactly 256 bytes
} QHeader256;

// Sector-aligned 256-byte paired descriptor header (QUdbHeader256)
// UDB = User Defined Block (memory, cpu context, registers, IoT state snapshots)
typedef struct {
    uint8_t  magic_paired[2];        // {'U', 'D'} (User Defined Block Descriptor)
    uint8_t  descriptor_version;     // Format version (1)
    uint8_t  descriptor_type;        // QUDB_BLOCK_* (Block Subtype)
    uint16_t sub_flags;              // Descriptor-specific feature flags
    uint16_t channel_or_bank;        // Sub-channel index, block index, or RAMBank ID
    uint64_t io_base_addr;           // Base I/O port address, memory aperture, or block base
    uint64_t io_window_len;          // Memory window size or register block span
    uint64_t param_registers[8];     // 8 x 64-bit context registers (CPU regs, baud, GPIO)
    char     property_map[64];       // Inline key-value properties parsed from {}
    uint64_t payload_slice_offset;   // Offset to block payload slice within archive (or 0 for inline/contiguous)
    uint64_t payload_slice_length;   // Length of block payload slice within archive
    uint8_t  custom_descriptor[88];  // Custom hardware descriptor bytes (exact 256 bytes total)
} QUdbHeader256;

#pragma pack(pop)

// API Functions
void qheader_init(QHeader256 *hdr, uint8_t payload_type, uint64_t seg, uint64_t off, uint64_t len);
void qudb_header_init(QUdbHeader256 *udb, uint8_t desc_type, uint16_t channel_or_bank, uint64_t base_addr, uint64_t win_len);
bool qheader_is_modern(const void *buffer, size_t size);
bool qheader_is_vintage(const void *buffer, size_t size);
uint32_t qheader_calc_crc32(const uint8_t *data, size_t length);
uint32_t qheader_crc32_update(uint32_t crc, const uint8_t *data, size_t length);
const char *qheader_resolve_extension(const char *filename, char *out_buf, size_t out_size);

#endif // STATEMENTS_FILESYSTEM_BINARY_OPS_QHEADER_H
