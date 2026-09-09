// FILENAME: qheader.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_qsave.c, stmt_qload.c, stmt_qrun.c, bload.c, bsave.c)
// NEEDS: libcore, libengine, libplatform
// Implements serialization, deserialization, and CRC-32 for binary package headers.

#include "statements/filesystem/binary_ops/qheader.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "platform/platform.h"

void qheader_init(QHeader256 *hdr, uint8_t payload_type, uint64_t seg, uint64_t off, uint64_t len) {
    if (!hdr) return;
    runtime_memset(hdr, 0, sizeof(QHeader256));

    hdr->magic_primary = QHEADER_MAGIC_PRIMARY;
    hdr->magic_extended = QHEADER_MAGIC_EXTENDED;
    runtime_memcpy(hdr->format_id, QHEADER_FORMAT_ID, 6);
    hdr->header_version = QHEADER_VERSION_CURRENT;

    // Auto-detect host architecture bitness (32-bit IoT vs 64-bit Desktop)
    hdr->arch_bits = (uint8_t)(sizeof(void *) * 8);
    hdr->endianness = 0; // 0 = Little-endian
    hdr->payload_type = payload_type;

    hdr->target_arch = QARCH_UNIVERSAL_VM;
    hdr->target_subsystem = QSUBSYS_IBM_PC_BIOS;

    hdr->segment_address = seg;
    hdr->offset_address = off;
    hdr->block_length = len;
    hdr->entry_point = off;
    hdr->uncompressed_length = len;
    hdr->crc32 = 0;
    hdr->flags = 0;
    hdr->chain_count = 0;
    hdr->paired_header_type = QUD_TYPE_GENERIC;
    hdr->export_count = 0;
    hdr->export_table_offset = 0;

    runtime_memcpy(hdr->module_name, "BASIC++ Binary Package\0", 23);
    runtime_memcpy(hdr->compiler_sig, "BASIC++ v6.5.2 bppc\0", 20);
}

void qudb_header_init(QUdbHeader256 *udb, uint8_t desc_type, uint16_t channel_or_bank, uint64_t base_addr, uint64_t win_len) {
    if (!udb) return;
    runtime_memset(udb, 0, sizeof(QUdbHeader256));

    udb->magic_paired[0] = 'U';
    udb->magic_paired[1] = 'D';
    udb->descriptor_version = 1;
    udb->descriptor_type = desc_type;
    udb->channel_or_bank = channel_or_bank;
    udb->io_base_addr = base_addr;
    udb->io_window_len = win_len;
    udb->payload_slice_offset = 0;
    udb->payload_slice_length = win_len;
}

bool qheader_is_modern(const void *buffer, size_t size) {
    if (!buffer || size < sizeof(QHeader256)) return false;
    const uint8_t *b = (const uint8_t *)buffer;
    if (b[0] != QHEADER_MAGIC_PRIMARY || b[1] != QHEADER_MAGIC_EXTENDED) {
        return false;
    }
    if (runtime_strncmp((const char *)&b[2], "QB256", 5) != 0) {
        return false;
    }
    return true;
}

bool qheader_is_vintage(const void *buffer, size_t size) {
    if (!buffer || size < sizeof(BSaveHeader)) return false;
    const uint8_t *b = (const uint8_t *)buffer;
    if (b[0] != QHEADER_MAGIC_PRIMARY) return false;
    if (size >= sizeof(QHeader256) && b[1] == QHEADER_MAGIC_EXTENDED) {
        if (runtime_strncmp((const char *)&b[2], "QB256", 5) == 0) {
            return false; // It's modern, not vintage
        }
    }
    return true;
}

uint32_t qheader_crc32_update(uint32_t crc, const uint8_t *data, size_t length) {
    if (!data || length == 0) return crc;
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        crc ^= byte;
        for (int j = 0; j < 8; j++) {
            uint32_t mask = -(int)(crc & 1U);
            crc = (crc >> 1) ^ (0xEDB88320U & mask);
        }
    }
    return crc;
}

uint32_t qheader_calc_crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFU;
    if (!data || length == 0) return 0;
    crc = qheader_crc32_update(crc, data, length);
    return ~crc;
}

static bool local_file_exists(const char *path) {
    if (!path) return false;
    void *h = platform_file_open(path, "rb");
    if (h) {
        platform_file_close(h);
        return true;
    }
    return false;
}

const char *qheader_resolve_extension(const char *filename, char *out_buf, size_t out_size) {
    if (!filename || !out_buf || out_size == 0) return filename;
    runtime_strncpy(out_buf, filename, out_size - 1);
    out_buf[out_size - 1] = '\0';

    // If file already has an extension, keep it
    const char *dot = runtime_strrchr(out_buf, '.');
    if (dot) return out_buf;

    // Check if target file exists as-is
    if (local_file_exists(out_buf)) return out_buf;

    // Permissive search taxonomy: .qbo, .iot, .qlb, .plg, .bin, .rom, .udb
    static const char *kExtensions[] = { ".qbo", ".iot", ".qlb", ".plg", ".bin", ".rom", ".udb" };
    size_t base_len = runtime_strlen(out_buf);

    for (size_t i = 0; i < sizeof(kExtensions) / sizeof(kExtensions[0]); i++) {
        if (base_len + runtime_strlen(kExtensions[i]) < out_size) {
            char test_path[512];
            runtime_strncpy(test_path, out_buf, sizeof(test_path) - 1);
            runtime_strncat(test_path, kExtensions[i], sizeof(test_path) - runtime_strlen(test_path) - 1);
            if (local_file_exists(test_path)) {
                runtime_strncpy(out_buf, test_path, out_size - 1);
                out_buf[out_size - 1] = '\0';
                return out_buf;
            }
        }
    }

    return out_buf;
}

// ---- Header Layout Table ----
// 256-Byte Primary Header (QHeader256):
// [00..01] Magic (0xFD, 0xFE)
// [02..07] Format ID ("QB256\0")
// [08..0B] Version (1), Bitness (16/32/64), Endianness (0/1), Payload Type
// [0C..0F] Target Arch (16-bit), Target Subsystem (16-bit, 0x01=PC BIOS, 0x10=IoT)
// [10..2F] Seg Addr (u64), Off Addr (u64), Block Len (u64), Entry Point (u64)
// [30..3B] Uncompressed Len (u64), CRC-32 Checksum (u32)
// [3C..4B] Flags (u32), Chain Count (u16), Paired Type (u16), Export Count (u32), Export Offset (u64)
// [4C..5F] Reserved Metadata (16 bytes)
// [60..7F] Module Name (32 bytes ASCII)
// [80..9F] Compiler Signature (32 bytes ASCII)
// [A0..FF] Reserved Padding (96 bytes) -> Total = Exactly 256 Bytes
//
// 256-Byte Paired Descriptor Header (QUdbHeader256):
// [00..01] Magic ('U', 'D' - User Defined Block Descriptor)
// [02..03] Descriptor Version (1), Descriptor Type (QUDB_BLOCK_*)
// [04..07] Sub Flags (16-bit), Channel or RAMBank ID (16-bit)
// [08..17] IO Base Address (u64), IO Window Length (u64)
// [18..57] Param Registers 0..7 (8 x u64 = 64 bytes)
// [58..97] Property Map (64 bytes inline JSON/properties parsed from {})
// [98..A7] Payload Slice Offset (u64), Payload Slice Length (u64)
// [A8..FF] Custom Hardware Descriptor / Pinout Padding (88 bytes) -> Total = Exactly 256 Bytes

