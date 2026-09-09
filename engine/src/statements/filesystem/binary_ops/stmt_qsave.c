// FILENAME: stmt_qsave.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the QSAVE statement in BASIC++.

#include "statements/filesystem/binary_ops/stmt_qsave.h"
#include "statements/filesystem/binary_ops/qheader.h"
#include "runtime/language_descriptor.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "platform/platform.h"
#include "memory/segmented_mem.h"
#include "bios/bios.h"
#include "statements/system/hardware/def_seg.h"
#include "types/errors.h"

static const LangDesc g_qsave_desc = {
    .name = "QSAVE",
    .category = "File I/O & Memory",
    .syntax = "QSAVE filename$, offset [, length [, payload_type%]] [, {properties}]",
    .description = "Saves binary package using modern 256-byte sector-aligned header with zero-malloc streaming.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_qsave_register(void) {
    lang_desc_register(&g_qsave_desc);
}

BppError stmt_qsave_handler(VMContext *vm, LexerContext *lex) {
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

    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
    } else {
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_SYNTAX;
        err.message = "Expected comma after filename";
        return err;
    }

    BValue off_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), fn_val.as.string);
        return err;
    }

    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
    } else {
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_SYNTAX;
        err.message = "Expected comma after offset";
        return err;
    }

    BValue len_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), fn_val.as.string);
        return err;
    }

    uint8_t payload_type = QPAYLOAD_MEMORY_IMAGE;
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        if (lex_peek(lex).type != TOK_LBRACE && lex_peek(lex).type != TOK_LBRACKET) {
            BValue type_val = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                str_release(vm_get_str(vm), fn_val.as.string);
                return err;
            }
            payload_type = (uint8_t)type_val.as.number;
        }
    }

    uint64_t offset = (uint64_t)off_val.as.number;
    uint64_t length = (uint64_t)len_val.as.number;
    uint64_t seg = 0;
#ifndef BASIC_LITE_BUILD
    VMemContext *vmem = vm_get_vmem(vm);
    if (vmem) seg = (uint64_t)vmem_get_def_seg(vmem);
#endif
    if (seg == 0) {
        seg = (uint64_t)runtime_get_def_seg();
    }

    // Check for optional paired descriptor property map ({})
    QUdbHeader256 udb;
    bool has_paired = false;
    qudb_header_init(&udb, QUD_TYPE_GENERIC, 0, offset, length);

    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
    }

    if (lex_peek(lex).type == TOK_LBRACE) {
        has_paired = true;
        lex_next(lex);
        while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF) {
            BppToken k = lex_next(lex);
            if (lex_peek(lex).start && *lex_peek(lex).start == ':') lex_next(lex);
            BppToken v = lex_next(lex);
            char k_str[32] = {0};
            char v_str[64] = {0};
            runtime_snprintf(k_str, sizeof(k_str), "%.*s", (int)k.length, k.start);
            runtime_snprintf(v_str, sizeof(v_str), "%.*s", (int)v.length, v.start);

            if (runtime_strcasestr(k_str, "wake")) {
                udb.param_registers[0] = (uint64_t)runtime_atoi(v_str);
            } else if (runtime_strcasestr(k_str, "baud")) {
                udb.param_registers[1] = (uint64_t)runtime_atoi(v_str);
            } else if (runtime_strcasestr(k_str, "bank")) {
                udb.channel_or_bank = (uint16_t)runtime_atoi(v_str);
                seg = (uint64_t)udb.channel_or_bank;
            } else if (runtime_strcasestr(k_str, "type")) {
                if (runtime_strcasestr(v_str, "cpu")) {
                    udb.descriptor_type = QUDB_BLOCK_CPU_CONTEXT;
                } else if (runtime_strcasestr(v_str, "mem")) {
                    udb.descriptor_type = QUDB_BLOCK_MEMORY;
                } else if (runtime_strcasestr(v_str, "rambank")) {
                    udb.descriptor_type = QUDB_BLOCK_RAMBANK;
                } else if (runtime_strcasestr(v_str, "io")) {
                    udb.descriptor_type = QUDB_BLOCK_IO_REGISTERS;
                } else if (runtime_strcasestr(v_str, "rom")) {
                    udb.descriptor_type = QUDB_BLOCK_ROM_WINDOW;
                } else if (runtime_strcasestr(v_str, "state")) {
                    udb.descriptor_type = QUDB_BLOCK_IOT_STATE;
                }
            }

            runtime_strncat(udb.property_map, k_str, sizeof(udb.property_map) - runtime_strlen(udb.property_map) - 1);
            runtime_strncat(udb.property_map, "=", sizeof(udb.property_map) - runtime_strlen(udb.property_map) - 1);
            runtime_strncat(udb.property_map, v_str, sizeof(udb.property_map) - runtime_strlen(udb.property_map) - 1);
            runtime_strncat(udb.property_map, ";", sizeof(udb.property_map) - runtime_strlen(udb.property_map) - 1);

            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RBRACE) lex_next(lex);
    }

    const char *path = str_data(fn_val.as.string);
    void *fp = platform_file_open(path, "wb");
    if (!fp) {
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_PERMISSION_DENIED;
        return err;
    }

    // Zero-malloc stack chunk buffer (512 bytes = 1 sector)
    uint8_t chunk[512];
    uint32_t raw_crc = 0xFFFFFFFFU;

    // Pass 1: Compute CRC-32 via zero-malloc stack streaming
    uint64_t remaining = length;
    uint64_t cur_off = offset;
    while (remaining > 0) {
        size_t chunk_len = (remaining > sizeof(chunk)) ? sizeof(chunk) : (size_t)remaining;
        for (size_t i = 0; i < chunk_len; i++) {
            uint8_t b = 0;
            uint32_t phys_addr = (uint32_t)((seg << 4) + (cur_off + i));
            if (vm_get_bios(vm)) {
                b = bios_peek(vm_get_bios(vm), phys_addr);
            }
#ifndef BASIC_LITE_BUILD
            else if (vmem) {
                vmem_peek(vmem, (uint16_t)(cur_off + i), &b);
            }
#endif
            chunk[i] = b;
        }
        raw_crc = qheader_crc32_update(raw_crc, chunk, chunk_len);
        cur_off += chunk_len;
        remaining -= chunk_len;
    }

    QHeader256 hdr;
    qheader_init(&hdr, payload_type, seg, offset, length);
    hdr.crc32 = (length > 0) ? (~raw_crc) : 0;

    if (payload_type == QPAYLOAD_IOT_FIRMWARE) {
        hdr.target_arch = QARCH_IOT_GENERIC;
        hdr.target_subsystem = QSUBSYS_FREESTANDING_IOT;
        hdr.flags |= QFLAG_AUTORUN_BOOT;
    } else if (payload_type == QPAYLOAD_OPTION_ROM_8086) {
        hdr.target_arch = QARCH_X86_REAL_8086;
        hdr.target_subsystem = QSUBSYS_IBM_PC_BIOS;
    } else if (payload_type == QPAYLOAD_RAMBANK_IMAGE) {
        hdr.target_subsystem = QSUBSYS_IBM_PC_BIOS;
    }

    if (has_paired) {
        hdr.flags |= QFLAG_HAS_PAIRED_HEADER;
        hdr.chain_count = 1;
        hdr.paired_header_type = udb.descriptor_type;
        if (udb.descriptor_type == QUDB_BLOCK_MEMORY || udb.descriptor_type == QUDB_BLOCK_RAMBANK || udb.descriptor_type == QUDB_BLOCK_CPU_CONTEXT) {
            hdr.flags |= QFLAG_HAS_UDB_MAP;
        }
    }

    platform_file_write(fp, &hdr, sizeof(hdr));

    if (has_paired) {
        platform_file_write(fp, &udb, sizeof(udb));
    }

    // Pass 2: Stream memory payload directly to file in 512-byte stack chunks
    remaining = length;
    cur_off = offset;
    while (remaining > 0) {
        size_t chunk_len = (remaining > sizeof(chunk)) ? sizeof(chunk) : (size_t)remaining;
        for (size_t i = 0; i < chunk_len; i++) {
            uint8_t b = 0;
            uint32_t phys_addr = (uint32_t)((seg << 4) + (cur_off + i));
            if (vm_get_bios(vm)) {
                b = bios_peek(vm_get_bios(vm), phys_addr);
            }
#ifndef BASIC_LITE_BUILD
            else if (vmem) {
                vmem_peek(vmem, (uint16_t)(cur_off + i), &b);
            }
#endif
            chunk[i] = b;
        }
        platform_file_write(fp, chunk, chunk_len);
        cur_off += chunk_len;
        remaining -= chunk_len;
    }

    platform_file_close(fp);
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

