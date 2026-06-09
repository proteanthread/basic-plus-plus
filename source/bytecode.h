/*
 * =====================================================================
 * BASIC++ Interpreter - bytecode.h
 * =====================================================================
 *
 * Bytecode format (.BPP) - Phase 13.
 *
 * PURPOSE:
 *   Defines the portable binary format for serializing and
 *   deserializing BASIC++ programs. The .bpp format stores
 *   tokenized program lines in a version-tagged container,
 *   matching how classic BASIC systems (TRS-80, Commodore)
 *   stored programs in binary.
 *
 * CONTAINER FORMAT:
 *   [16-byte header]
 *   [line 0: 2-byte linenum LE, 2-byte textlen LE, N-byte text]
 *   [line 1: ...]
 *   ...
 *
 *   The text is stored as raw source (not tokenized into bytecodes)
 *   because the interpreter's Lexer re-tokenizes on each execution.
 *   This keeps the format simple and maximally compatible - any
 *   BASIC++ version can load any .bpp file as long as the header
 *   version is recognized.
 *
 * HEADER (16 bytes):
 *   Bytes 0-3:  Magic "BPP\x1A" (4 bytes, \x1A = EOF marker)
 *   Byte  4:    Format version (1)
 *   Byte  5:    Dialect ID that created this file
 *   Bytes 6-7:  Flags (reserved, 0)
 *   Bytes 8-9:  Line count (little-endian uint16)
 *   Bytes 10-15: Reserved (0)
 *
 * LINE RECORD (variable length):
 *   Bytes 0-1:  Line number (little-endian uint16)
 *   Bytes 2-3:  Text length (little-endian uint16, excl NUL)
 *   Bytes 4+:   Source text (NOT NUL-terminated in file)
 *
 * BYTE ORDER: Little-endian throughout.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_BYTECODE_H
#define BASICPP_BYTECODE_H

#include "memory.h"

/* =====================================================================
 * Constants
 * =====================================================================
 */
#define BPP_MAGIC_0     'B'
#define BPP_MAGIC_1     'P'
#define BPP_MAGIC_2     'P'
#define BPP_MAGIC_3     '\x1A'   /* Ctrl-Z EOF marker */
#define BPP_FORMAT_VER  1
#define BPP_HEADER_SIZE 16

/* =====================================================================
 * BPP Header Structure
 * =====================================================================
 * Stored at the start of every .bpp file. Exactly 16 bytes.
 * All multi-byte values are little-endian.
 */
typedef struct BppHeader {
    unsigned char magic[4];       /* "BPP\x1A" */
    unsigned char version;        /* format version (1) */
    unsigned char dialect;        /* DialectId that created this */
    unsigned char flags[2];       /* reserved (0) */
    unsigned char line_count[2];  /* little-endian line count */
    unsigned char reserved[6];    /* padding to 16 bytes */
} BppHeader;

/* =====================================================================
 * Bytecode API
 * =====================================================================
 */

/*
 * bpp_save - Serialize the program store to a .bpp file.
 *
 * Writes the BPP header followed by each program line as a
 * length-prefixed text record.
 *
 * Parameters:
 *   prog     - program store to serialize
 *   filename - output file path (e.g., "program.bpp")
 *
 * Returns 0 on success, -1 on error (prints message).
 */
int bpp_save(const ProgramStore *prog, const char *filename);

/*
 * bpp_load - Deserialize a .bpp file into the program store.
 *
 * Reads and validates the BPP header, then loads each line
 * record into the program store. The program store is cleared
 * before loading (like NEW + LOAD).
 *
 * Parameters:
 *   prog     - program store to populate
 *   filename - input file path (e.g., "program.bpp")
 *
 * Returns 0 on success, -1 on error (prints message).
 */
int bpp_load(ProgramStore *prog, const char *filename);

#endif /* BASICPP_BYTECODE_H */
