/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: archive_core.c
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
 * File: archive_core.c
 * ===================================================================== */

#include "archive_core.h"
#include <string.h>

void archive_core_write_le16(unsigned char *buf, unsigned int val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
}

unsigned int archive_core_read_le16(const unsigned char *buf)
{
    return (unsigned int)buf[0] |
           ((unsigned int)buf[1] << 8);
}

void archive_core_write_le32(unsigned char *buf, unsigned long val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
    buf[2] = (unsigned char)((val >> 16) & 0xFF);
    buf[3] = (unsigned char)((val >> 24) & 0xFF);
}

unsigned long archive_core_read_le32(const unsigned char *buf)
{
    return (unsigned long)buf[0] |
           ((unsigned long)buf[1] << 8) |
           ((unsigned long)buf[2] << 16) |
           ((unsigned long)buf[3] << 24);
}

unsigned int archive_core_crc16(unsigned int seed, const unsigned char *data, int len)
{
    unsigned int crc = seed;
    int i, j;
    for (i = 0; i < len; i++) {
        crc ^= (unsigned int)data[i] << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc = crc << 1;
            crc &= 0xFFFF;
        }
    }
    return crc;
}
