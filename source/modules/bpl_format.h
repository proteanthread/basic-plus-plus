/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bpl_format.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    C-level modular expansions providing hardware wrappers and runtime libraries.
 *
 * 2. WHAT TO EXPECT:
 *    Modules register customized functions at boot to dynamically extend vocabulary.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Module naming, registered commands list, setup/shutdown details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Module lifecycle dispatcher, keyword override bindings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure mod_init does not fail. Verify that linkage matches build profiles.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - bpl_format.h
 // ---
 //
 // .BPL (BASIC++ Portable Library) file format.
 //
 // PURPOSE:
 // OS-independent binary container for pre-compiled library
 // bytecode. Stores the instruction stream, symbol table,
 // and string pool in a portable, little-endian format.
 //
 // .BPL files can be distributed without source code and
 // loaded faster than compiling from source at runtime.
 //
 // FILE LAYOUT:
 //   [32-byte header]
 //   [symbol table]
 //   [instruction stream]
 //   [string pool]
 //
 // HEADER (32 bytes):
 //   0-3:    Magic "BPL\x1A"
 //   4:      Format version (1)
 //   5:      Security level (required_level)
 //   6:      Extension type (LIB/FN/FT/MOD/PLG)
 //   7:      Flags (reserved)
 //   8-9:    Symbol count (LE16)
 //   10-11:  Instruction count (LE16)
 //   12-13:  String pool size (LE16)
 //   14-15:  Source checksum (CRC16)
 //   16-31:  Library name (NUL-padded, 16 chars max)
 //
 // SYMBOL TABLE ENTRY (variable-length per entry):
 //   1 byte:   type (0=SUB, 1=FUNCTION, 2=DEF_FN)
 //   1 byte:   param count
 //   2 bytes:  instruction offset (LE16)
 //   1 byte:   name length
 //   N bytes:  name (ASCII, no NUL)
 //
 // INSTRUCTION STREAM (6 bytes per instruction):
 //   1 byte:   opcode (PCodeOp enum)
 //   1 byte:   operand type (0=none,1=int,2=float,3=str_ref)
 //   4 bytes:  operand value (LE32)
 //
 // STRING POOL:
 //   Packed NUL-terminated strings.
 //
 // All multi-byte values are little-endian for portability.
 //
 // C89/C90 COMPLIANT.
 //
 // ---

#ifndef BASICPP_BPL_FORMAT_H
#define BASICPP_BPL_FORMAT_H

#include "lib_space.h"

// --- Constants ---
#define BPL_MAGIC_0     'B'
#define BPL_MAGIC_1     'P'
#define BPL_MAGIC_2     'L'
#define BPL_MAGIC_3     '\x1A' // Ctrl-Z EOF marker
#define BPL_FORMAT_VER  1
#define BPL_HEADER_SIZE 32
#define BPL_INSTR_SIZE  6 // 1 op + 1 type + 4 value

// --- Operand Type Tags (for serialization) ---
#define BPL_OP_NONE     0
#define BPL_OP_INT      1
#define BPL_OP_FLOAT    2
#define BPL_OP_STR_REF  3
#define BPL_OP_OFFSET   4

// --- Obfuscation Constants ---
#define BPL_FLAG_OBFUSCATED 0x01
#define BPL_XOR_KEY         0x5A

// --- BPL Header (in-memory representation) ---
typedef struct BplHeader {
    unsigned char magic[4]; // "BPL\x1A"
    unsigned char version; // format version (1)
    unsigned char security_level; // required SecLevel
    unsigned char ext_type; // LibExtType
    unsigned char flags; // reserved
    unsigned char symbol_count[2]; // LE16
    unsigned char instr_count[2]; // LE16
    unsigned char str_pool_size[2]; // LE16
    unsigned char checksum[2]; // CRC16 of source
    char          lib_name[16]; // NUL-padded
} BplHeader;

// ===================================================================
 // BPL FORMAT API
 // ===================================================================

 // bpl_save - Serialize a compiled library to .BPL file.
 //
 // Writes header, symbol table, instruction stream, and
 // string pool to a binary file. All values are little-endian.
 //
 // Parameters:
 //   lib      - compiled library to serialize
 //   filename - output file path (e.g., "turtle.bpl")
 //
 // Returns 0 on success, -1 on error.
int bpl_save(const LoadedLibrary *lib, const char *filename);

 // bpl_load - Deserialize a .BPL file into a library slot.
 //
 // Reads and validates the header, then loads the symbol
 // table, instruction stream, and string pool.
 //
 // Parameters:
 //   filename - input file path (e.g., "turtle.bpl")
 //   lib      - library slot to populate
 //
 // Returns 0 on success, -1 on error.
int bpl_load(const char *filename, LoadedLibrary *lib);

 // bpl_crc16 - Compute CRC-16/CCITT checksum.
 //
 // Used to verify .BPL integrity and match compiled
 // bytecode to its source file.
 //
 // Parameters:
 //   data - byte buffer
 //   len  - buffer length
 //
 // Returns 16-bit CRC value.
unsigned int bpl_crc16(const unsigned char *data, int len);

#endif // BASICPP_BPL_FORMAT_H
