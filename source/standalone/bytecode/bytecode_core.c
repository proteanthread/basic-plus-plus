/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bytecode_core.c
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
 * File: bytecode_core.c
 * ===================================================================== */

#include "bytecode_core.h"
#include <string.h>

void bytecode_core_write_le16(unsigned char *buf, unsigned int val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
}

unsigned int bytecode_core_read_le16(const unsigned char *buf)
{
    return (unsigned int)buf[0] |
           ((unsigned int)buf[1] << 8);
}

void bytecode_core_write_le32(unsigned char *buf, unsigned long val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
    buf[2] = (unsigned char)((val >> 16) & 0xFF);
    buf[3] = (unsigned char)((val >> 24) & 0xFF);
}

unsigned long bytecode_core_read_le32(const unsigned char *buf)
{
    return (unsigned long)buf[0] |
           ((unsigned long)buf[1] << 8) |
           ((unsigned long)buf[2] << 16) |
           ((unsigned long)buf[3] << 24);
}

void bytecode_core_write_le64(unsigned char *buf, unsigned long long val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
    buf[2] = (unsigned char)((val >> 16) & 0xFF);
    buf[3] = (unsigned char)((val >> 24) & 0xFF);
    buf[4] = (unsigned char)((val >> 32) & 0xFF);
    buf[5] = (unsigned char)((val >> 40) & 0xFF);
    buf[6] = (unsigned char)((val >> 48) & 0xFF);
    buf[7] = (unsigned char)((val >> 56) & 0xFF);
}

unsigned long long bytecode_core_read_le64(const unsigned char *buf)
{
    return (unsigned long long)buf[0] |
           ((unsigned long long)buf[1] << 8) |
           ((unsigned long long)buf[2] << 16) |
           ((unsigned long long)buf[3] << 24) |
           ((unsigned long long)buf[4] << 32) |
           ((unsigned long long)buf[5] << 40) |
           ((unsigned long long)buf[6] << 48) |
           ((unsigned long long)buf[7] << 56);
}

void bytecode_core_write_double(unsigned char *buf, double val)
{
    union {
        double d;
        unsigned long long u;
    } temp;
    temp.d = val;
    bytecode_core_write_le64(buf, temp.u);
}

double bytecode_core_read_double(const unsigned char *buf)
{
    union {
        double d;
        unsigned long long u;
    } temp;
    temp.u = bytecode_core_read_le64(buf);
    return temp.d;
}

unsigned int bytecode_core_crc16(unsigned int seed, const unsigned char *data, int len)
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
