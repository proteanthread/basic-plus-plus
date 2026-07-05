/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bpl_format_core.h
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
 * File: bpl_format_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_BPL_FORMAT_CORE_H
#define BASICPP_STANDALONE_BPL_FORMAT_CORE_H

#define BPL_MAGIC_0     'B'
#define BPL_MAGIC_1     'P'
#define BPL_MAGIC_2     'L'
#define BPL_MAGIC_3     '\x1A'
#define BPL_FORMAT_VER  1
#define BPL_HEADER_SIZE 32
#define BPL_INSTR_SIZE  6

#define BPL_FLAG_OBFUSCATED 0x01
#define BPL_XOR_KEY         0x5A

void bpl_core_write_le16(unsigned char *buf, unsigned int val);
unsigned int bpl_core_read_le16(const unsigned char *buf);
unsigned int bpl_core_crc16(const unsigned char *data, int len);
unsigned int bpl_core_crc16_update(unsigned int seed, const unsigned char *data, int len);
void bpl_core_xor_buffer(unsigned char *data, int len);

#endif // BASICPP_STANDALONE_BPL_FORMAT_CORE_H
