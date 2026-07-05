/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: archive_core.h
 * Subsystem: Single Binary Executable Archiver
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Manages package directories, CRC-16 generation, and extraction of BPE compiled archives.
 *
 * 2. WHAT TO EXPECT:
 *    Loads files, validates header magic, calculates CRC checksums.
 *
 * 3. WHAT CAN BE CHANGED:
 *    XOR keys, signature verification codes.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BPE binary file format specification.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If package verification fails, check file integrity and file size.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE BPE EXECUTION ARCHIVER ENGINE
 * File: archive_core.h
 * ===================================================================== */

#ifndef STANDALONE_ARCHIVE_CORE_H
#define STANDALONE_ARCHIVE_CORE_H

#include <stddef.h>

#define BPE_MAGIC "BPE\x1A"
#define BPE_NEW_MAGIC "BPP\x1B"

#pragma pack(push, 1)
typedef struct BpeHeader {
    unsigned char magic[4];       // "BPE\x1A"
    unsigned char format_ver;     // format version (1)
    unsigned char chunk_count;    // number of chunks in directory
    unsigned char header_crc[2];  // little-endian CRC-16 of header+directory
    unsigned char total_size[4];  // little-endian total file size
    unsigned char reserved[4];    // padding
} BpeHeader;

typedef struct BpeChunkDescriptor {
    char type[4];                 // "META", "DEPS", "SRC_", "BYTE", "SIGN"
    unsigned char offset[4];      // little-endian absolute byte offset
    unsigned char length[4];      // little-endian size of payload in bytes
    unsigned char reserved[4];    // padding
} BpeChunkDescriptor;
#pragma pack(pop)

/* Core packing & CRC functions */
unsigned int archive_core_crc16(unsigned int seed, const unsigned char *data, int len);

void archive_core_write_le16(unsigned char *buf, unsigned int val);
unsigned int archive_core_read_le16(const unsigned char *buf);
void archive_core_write_le32(unsigned char *buf, unsigned long val);
unsigned long archive_core_read_le32(const unsigned char *buf);

#endif /* STANDALONE_ARCHIVE_CORE_H */
