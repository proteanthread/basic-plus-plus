/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bytecode_core.h
 * Subsystem: P-Code Bytecode Program Format Serializer
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Serializes and deserializes compiled bytecode instructions to/from .BPP files.
 *
 * 2. WHAT TO EXPECT:
 *    Quick load/save of instruction blocks and constant string pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Header signatures, maximum file limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Bytecode instruction structure fields.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If program load fails, check bytecode version compatibility.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE BYTECODE SERIALIZATION ENGINE
 * File: bytecode_core.h
 * ===================================================================== */

#ifndef STANDALONE_BYTECODE_CORE_H
#define STANDALONE_BYTECODE_CORE_H

#include <stddef.h>

#define BPP_MAGIC_0 'B'
#define BPP_MAGIC_1 'P'
#define BPP_MAGIC_2 'P'
#define BPP_MAGIC_3 '\x1A' // Ctrl-Z EOF marker
#define BPP_FORMAT_VER 1
#define BPP_HEADER_SIZE 16

#define BPP_NEW_MAGIC "BPP\x1B"
#define BPP_NEW_FORMAT_VER 3

typedef struct BppNewHeader {
    unsigned char magic[4];          // "BPP\x1B"
    unsigned char format_ver;        // format version (2)
    unsigned char vm_major;          // target VM major version
    unsigned char vm_minor;          // target VM minor version
    unsigned char vm_patch;          // target VM patch version
    unsigned char instruction_ver[2]; // instruction set version (LE)
    unsigned char dialect_id;        // Dialect ID
    unsigned char dependency_count[2]; // number of required modules (LE)
    unsigned char flags[2];          // flags (LE)
    unsigned char instr_count[4];     // number of compiled instructions (LE)
    unsigned char str_pool_size[4];   // size of string constant pool (LE)
    unsigned char on_table_size[4];   // size of ON GOTO/GOSUB jump tables (LE)
    unsigned char source_checksum[4]; // CRC-16 of original source text lines (LE)
    unsigned char crc_checksum[2];    // CRC-16 integrity of the payload (LE)
} BppNewHeader;

typedef struct BppDependency {
    char name[32];
    unsigned char version_major;
    unsigned char version_minor;
    unsigned char reserved[2];
} BppDependency;

/* Core utilities */
unsigned int bytecode_core_crc16(unsigned int seed, const unsigned char *data, int len);

void bytecode_core_write_le16(unsigned char *buf, unsigned int val);
unsigned int bytecode_core_read_le16(const unsigned char *buf);
void bytecode_core_write_le32(unsigned char *buf, unsigned long val);
unsigned long bytecode_core_read_le32(const unsigned char *buf);
void bytecode_core_write_le64(unsigned char *buf, unsigned long long val);
unsigned long long bytecode_core_read_le64(const unsigned char *buf);
void bytecode_core_write_double(unsigned char *buf, double val);
double bytecode_core_read_double(const unsigned char *buf);

#endif /* STANDALONE_BYTECODE_CORE_H */
