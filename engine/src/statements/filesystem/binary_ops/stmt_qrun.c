// FILENAME: stmt_qrun.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the QRUN statement in BASIC++.

#include "statements/filesystem/binary_ops/stmt_qrun.h"
#include "statements/filesystem/binary_ops/qheader.h"
#include "runtime/language_descriptor.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "platform/platform.h"
#include "memory/segmented_mem.h"
#include "bios/bios.h"
#include "statements/system/hardware/def_seg.h"
#include "types/errors.h"

static const LangDesc g_qrun_desc = {
    .name = "QRUN",
    .category = "File I/O & Execution",
    .syntax = "QRUN filename$ [, param$]",
    .description = "Multi-modal super-runner for modern 256-byte binary packages, plugins, and vintage images with zero-malloc streaming.",
    .error_summary = "Error 2: Syntax Error, Error 53: File Not Found, Error 54: Bad File Mode",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_qrun_register(void) {
    lang_desc_register(&g_qrun_desc);
}

BppError stmt_qrun_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    BValue fn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (fn_val.type != VAL_STRING || !fn_val.as.string) {
        if (fn_val.type == VAL_STRING && fn_val.as.string) {
            str_release(vm_get_str(vm), fn_val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    // Optional argument or execution address parameter
    bool has_param = false;
    BValue param_val;
    runtime_memset(&param_val, 0, sizeof(param_val));
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        param_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            str_release(vm_get_str(vm), fn_val.as.string);
            return err;
        }
        has_param = true;
    }

    const char *path = str_data(fn_val.as.string);
    char resolved_path[512];
    path = qheader_resolve_extension(path, resolved_path, sizeof(resolved_path));

    void *fp = platform_file_open(path, "rb");
    if (!fp) {
        if (has_param && param_val.type == VAL_STRING && param_val.as.string) {
            str_release(vm_get_str(vm), param_val.as.string);
        }
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }

    uint8_t peek_buf[256];
    size_t read_bytes = platform_file_read(fp, peek_buf, sizeof(peek_buf));
    if (read_bytes < 7) {
        platform_file_close(fp);
        if (has_param && param_val.type == VAL_STRING && param_val.as.string) {
            str_release(vm_get_str(vm), param_val.as.string);
        }
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_BAD_FILE_MODE;
        err.message = "File too short for binary package";
        return err;
    }

    uint64_t seg = 0;
#ifndef BASIC_LITE_BUILD
    VMemContext *vmem = vm_get_vmem(vm);
    if (vmem) seg = (uint64_t)vmem_get_def_seg(vmem);
#endif
    if (seg == 0) {
        seg = (uint64_t)runtime_get_def_seg();
    }

    if (qheader_is_modern(peek_buf, read_bytes)) {
        const QHeader256 *hdr = (const QHeader256 *)peek_buf;
        uint64_t length = hdr->block_length;
        uint64_t target_offset = hdr->offset_address;
        if (hdr->segment_address != 0) {
            seg = hdr->segment_address;
        }

        // Process chained 256-byte UDB descriptor headers (User Defined Blocks)
        uint16_t num_udb = (hdr->chain_count > 0) ? hdr->chain_count : ((hdr->flags & QFLAG_HAS_PAIRED_HEADER) ? 1 : 0);
        QUdbHeader256 udb;
        for (uint16_t ci = 0; ci < num_udb; ci++) {
            size_t udb_read = platform_file_read(fp, &udb, sizeof(udb));
            if (udb_read == sizeof(udb)) {
                if (udb.channel_or_bank > 0) {
                    seg = (uint64_t)udb.channel_or_bank;
                }
                if (udb.descriptor_type == QUDB_BLOCK_MEMORY || udb.descriptor_type == QUDB_BLOCK_RAMBANK) {
                    if (udb.io_base_addr > 0) {
                        target_offset = udb.io_base_addr;
                    }
                }
            }
        }

        // Stream payload via zero-malloc stack chunk buffer (512 bytes)
        if (length > 0) {
            uint8_t chunk[512];
            uint32_t raw_crc = 0xFFFFFFFFU;
            uint64_t remaining = length;
            uint64_t cur_off = target_offset;

            while (remaining > 0) {
                size_t to_read = (remaining > sizeof(chunk)) ? sizeof(chunk) : (size_t)remaining;
                size_t actual_read = platform_file_read(fp, chunk, to_read);
                if (actual_read < to_read) {
                    platform_file_close(fp);
                    if (has_param && param_val.type == VAL_STRING && param_val.as.string) {
                        str_release(vm_get_str(vm), param_val.as.string);
                    }
                    str_release(vm_get_str(vm), fn_val.as.string);
                    err.code = ERR_BAD_FILE_MODE;
                    err.message = "Truncated payload in QRUN package";
                    return err;
                }

                raw_crc = qheader_crc32_update(raw_crc, chunk, to_read);

                // Load into target memory / BIOS / VMem
                for (size_t i = 0; i < to_read; i++) {
                    uint32_t phys_addr = (uint32_t)((seg << 4) + (cur_off + i));
                    if (vm_get_bios(vm)) {
                        bios_poke(vm_get_bios(vm), phys_addr, chunk[i]);
                    }
#ifndef BASIC_LITE_BUILD
                    else if (vmem) {
                        vmem_poke(vmem, (uint16_t)(cur_off + i), chunk[i]);
                    }
#endif
                }

                cur_off += to_read;
                remaining -= to_read;
            }

            // CRC-32 Validation
            if (hdr->crc32 != 0) {
                uint32_t calc_crc = ~raw_crc;
                if (calc_crc != hdr->crc32) {
                    platform_file_close(fp);
                    if (has_param && param_val.type == VAL_STRING && param_val.as.string) {
                        str_release(vm_get_str(vm), param_val.as.string);
                    }
                    str_release(vm_get_str(vm), fn_val.as.string);
                    err.code = ERR_BAD_FILE_MODE;
                    err.message = "Corrupted QRUN package: CRC32 mismatch";
                    return err;
                }
            }
        }
    } else if (qheader_is_vintage(peek_buf, read_bytes)) {
        // Super-runner fallback: load and execute vintage 7-byte BSAVE format
        uint16_t hdr_offset = (uint16_t)(peek_buf[3] | (peek_buf[4] << 8));
        uint64_t length = (uint64_t)(peek_buf[5] | (peek_buf[6] << 8));
        uint64_t target_offset = (uint64_t)hdr_offset;

        platform_file_seek(fp, 7, PLATFORM_SEEK_SET);

        uint8_t chunk[512];
        uint64_t remaining = length;
        uint64_t cur_off = target_offset;

        while (remaining > 0) {
            size_t to_read = (remaining > sizeof(chunk)) ? sizeof(chunk) : (size_t)remaining;
            size_t actual_read = platform_file_read(fp, chunk, to_read);
            if (actual_read == 0) break;

            for (size_t i = 0; i < actual_read; i++) {
                uint32_t phys_addr = (uint32_t)((seg << 4) + (cur_off + i));
                if (vm_get_bios(vm)) {
                    bios_poke(vm_get_bios(vm), phys_addr, chunk[i]);
                }
#ifndef BASIC_LITE_BUILD
                else if (vmem) {
                    vmem_poke(vmem, (uint16_t)(cur_off + i), chunk[i]);
                }
#endif
            }

            cur_off += actual_read;
            remaining -= actual_read;
            if (actual_read < to_read) break;
        }
    } else {
        platform_file_close(fp);
        if (has_param && param_val.type == VAL_STRING && param_val.as.string) {
            str_release(vm_get_str(vm), param_val.as.string);
        }
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_BAD_FILE_MODE;
        err.message = "Invalid binary header format for QRUN";
        return err;
    }

    platform_file_close(fp);
    if (has_param && param_val.type == VAL_STRING && param_val.as.string) {
        str_release(vm_get_str(vm), param_val.as.string);
    }
    str_release(vm_get_str(vm), fn_val.as.string);
    return err;
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
// 256-Byte Paired Header (QUdbHeader256):
// [00..01] Magic ('U', 'D')
// [02..03] Version (1), Type (QUD_TYPE_*)
// [04..07] Sub-Flags (u16), Channel/Bank (u16)
// [08..17] I/O Base Addr (u64), Window Len (u64)
// [18..57] 8 x 64-bit Parameter Registers (64 bytes)
// [58..97] Property Map (64 bytes ASCII key-value pairs)
// [98..FF] Custom Hardware Descriptor (104 bytes) -> Total = Exactly 256 Bytes
