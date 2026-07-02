/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bytecode.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Stored program editor commands (RENUM, DELETE), compiler driver pipeline (BASIC-to-C), and bytecode serializers.
 *
 * 2. WHAT TO EXPECT:
 *    Code generator constructs self-contained C89 files. Bytecode serializes code to files.
 *
 * 3. WHAT CAN BE CHANGED:
 *    C89 codegen shims, editor warnings, target language mapping layout.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    AST translation loops, bytecode file format specs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify target C compiler settings. If transpiled C file has compilation warnings, check codegen expressions mapping.
 * ===================================================================== */

 // bytecode.h -- .bpp file format and serialization
 //
 // Native binary format for BASIC++ programs. Header + line records.
 // Stores source text (not tokenized opcodes) for maximum compatibility.
 //
 // .bpp container:
 //   [16-byte header]
 //   [line 0: 2-byte linenum LE, 2-byte textlen LE, N-byte text]
 //   [line 1: ...]
 //   ...
 //
 // Header (16 bytes):
 //   0-3:   Magic "BPP\x1A" (Ctrl-Z EOF marker)
 //   4:     Format version (1)
 //   5:     DialectId that created this file
 //   6-7:   Flags (reserved)
 //   8-9:   Line count (LE uint16)
 //   10-15: Reserved
 //
 // Building a detokenizer for older dialects:
 //
 //   GW-BASIC tokenized format (0xFF header):
 //     Byte 0: always 0xFF. Then linked-list of line records:
 //     [2-byte next-line offset LE] [2-byte line number LE]
 //     [tokenized body] [0x00 terminator]
 //     End of program: 0x00 0x00 (null next-line pointer).
 //     Tokens are single bytes 0x80-0xFF mapping to keywords.
 //     String literals and numbers are stored inline as ASCII.
 //     To detokenize: read each token byte, look up the keyword
 //     string in a table (same order as lexer.c keyword_table),
 //     and reconstruct the ASCII line.
 //
 //   Commodore BASIC (PRG format):
 //     First 2 bytes: load address (usually $0801 for C64).
 //     Then same linked-list structure as GW-BASIC but with
 //     PETSCII encoding and different token values (0x80-0xCB).
 //     Need a PETSCII-to-ASCII translation table on top of the
 //     token-to-keyword table.
 //
 //   Atari BASIC tokenized format:
 //     Completely different. Variable Name Table at the start,
 //     then Statement Table, then tokenized lines referencing
 //     variables by index. Harder to detokenize but documented
 //     in De Re Atari and the Atari BASIC Reference Manual.
 //
 //   To add detokenization to bpp_load(): detect the format from
 //   the first byte(s), call the appropriate detokenizer, then
 //   insert the resulting ASCII lines into the program store.
 //   Each detokenizer is independent -- no shared state needed.

#ifndef BASICPP_BYTECODE_H
#define BASICPP_BYTECODE_H

#include "memory.h"

// --- Constants ---
#define BPP_MAGIC_0 'B'
#define BPP_MAGIC_1 'P'
#define BPP_MAGIC_2 'P'
#define BPP_MAGIC_3 '\x1A' // Ctrl-Z EOF marker
#define BPP_FORMAT_VER 1
#define BPP_HEADER_SIZE 16

// --- BPP Header Structure ---
 // Stored at the start of every .bpp file. Exactly 16 bytes.
 // All multi-byte values are little-endian.
typedef struct BppHeader {
 unsigned char magic[4]; // "BPP\x1A"
 unsigned char version; // format version (1)
 unsigned char dialect; // DialectId that created this
 unsigned char flags[2]; // reserved (0)
 unsigned char line_count[2]; // little-endian line count
 unsigned char reserved[6]; // padding to 16 bytes
} BppHeader;

// --- Bytecode API ---

typedef int (*DetokenizerFn)(const unsigned char *data, int len, char *out_text, int max_out);
extern DetokenizerFn g_custom_detokenizer;
void bytecode_set_detokenizer(DetokenizerFn fn);

 // bpp_save - Serialize the program store to a .bpp file.
 //
 // Writes the BPP header followed by each program line as a
 // length-prefixed text record.
 //
 // Parameters:
 // prog - program store to serialize
 // filename - output file path (e.g., "program.bpp")
 //
 // Returns 0 on success, -1 on error (prints message).
int bpp_save(const ProgramStore *prog, const char *filename);

 // bpp_load - Deserialize a .bpp file into the program store.
 //
 // Reads and validates the BPP header, then loads each line
 // record into the program store. The program store is cleared
 // before loading (like NEW + LOAD).
 //
 // Parameters:
 // prog - program store to populate
 // filename - input file path (e.g., "program.bpp")
 //
 // Returns 0 on success, -1 on error (prints message).
int bpp_load(ProgramStore *prog, const char *filename, void *rt_ptr);

#endif // BASICPP_BYTECODE_H
