/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bpl_format_core.c
 * Subsystem: BPL Serialized Package Compiler Parser
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Encapsulates Endianness validation, CRC16 hashing, and XOR encryption loops.
 *
 * 2. WHAT TO EXPECT:
 *    Verifies binary files, handles byte order swaps.
 *
 * 3. WHAT CAN BE CHANGED:
 *    XOR mask arrays, header fields constraints.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BPL serialization byte protocol.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If validation fails, check compilation logs and platform architecture.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE BPL SERIALIZER CORE
 * File: bpl_format_core.c
 * ===================================================================== */

#include "bpl_format_core.h"

void bpl_core_write_le16(unsigned char *buf, unsigned int val)
{
    if (buf) {
        buf[0] = (unsigned char)(val & 0xFF);
        buf[1] = (unsigned char)((val >> 8) & 0xFF);
    }
}

unsigned int bpl_core_read_le16(const unsigned char *buf)
{
    if (!buf) return 0;
    return (unsigned int)buf[0] | ((unsigned int)buf[1] << 8);
}

unsigned int bpl_core_crc16_update(unsigned int seed, const unsigned char *data, int len)
{
    unsigned int crc = seed;
    if (data) {
        int i;
        for (i = 0; i < len; i++) {
            crc ^= (unsigned int)data[i] << 8;
            {
                int j;
                for (j = 0; j < 8; j++) {
                    if (crc & 0x8000) {
                        crc = (crc << 1) ^ 0x1021;
                    } else {
                        crc = crc << 1;
                    }
                    crc &= 0xFFFF;
                }
            }
        }
    }
    return crc;
}

unsigned int bpl_core_crc16(const unsigned char *data, int len)
{
    return bpl_core_crc16_update(0xFFFF, data, len);
}

void bpl_core_xor_buffer(unsigned char *data, int len)
{
    if (data) {
        int i;
        for (i = 0; i < len; i++) {
            data[i] ^= BPL_XOR_KEY;
        }
    }
}
