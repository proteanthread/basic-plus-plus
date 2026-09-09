// FILENAME: brun.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the BRUN statement in BASIC++.

#include "statements/filesystem/binary_ops/brun.h"
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

static const LangDesc g_brun_desc = {
    .name = "BRUN",
    .category = "File I/O & Execution",
    .syntax = "BRUN filename$ [, address%]",
    .description = "Loads a vintage 7-byte binary memory image into memory and executes it.",
    .error_summary = "Error 2: Syntax Error, Error 53: File Not Found, Error 54: Bad File Mode",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_brun_register(void) {
    lang_desc_register(&g_brun_desc);
}

BppError stmt_brun_handler(VMContext *vm, LexerContext *lex) {
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

    bool has_user_addr = false;
    uint16_t user_addr = 0;

    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        BValue addr_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            str_release(vm_get_str(vm), fn_val.as.string);
            return err;
        }
        user_addr = (uint16_t)addr_val.as.number;
        has_user_addr = true;
    }

    const char *path = str_data(fn_val.as.string);
    void *fp = platform_file_open(path, "rb");
    if (!fp) {
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }

    uint8_t hdr[7];
    size_t read_bytes = platform_file_read(fp, hdr, 7);
    if (read_bytes < 7 || hdr[0] != QHEADER_MAGIC_PRIMARY) {
        platform_file_close(fp);
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_BAD_FILE_MODE;
        err.message = "Invalid BSAVE header or bad file format";
        return err;
    }

    // Reject modern 256-byte files to protect vintage memory execution boundaries
    if (hdr[1] == QHEADER_MAGIC_EXTENDED) {
        platform_file_close(fp);
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_BAD_FILE_MODE;
        err.message = "Cannot BRUN modern 256-byte QSAVE package; use QRUN";
        return err;
    }

    uint16_t hdr_offset = (uint16_t)(hdr[3] | (hdr[4] << 8));
    uint16_t length = (uint16_t)(hdr[5] | (hdr[6] << 8));
    uint16_t target_offset = has_user_addr ? user_addr : hdr_offset;

    uint16_t seg = 0;
#ifndef BASIC_LITE_BUILD
    VMemContext *vmem = vm_get_vmem(vm);
    if (vmem) seg = vmem_get_def_seg(vmem);
#endif
    if (seg == 0) {
        seg = runtime_get_def_seg();
    }

    for (uint32_t i = 0; i < length; i++) {
        uint8_t b = 0;
        if (platform_file_read(fp, &b, 1) != 1) break;
        uint32_t phys_addr = ((uint32_t)seg << 4) + (uint32_t)(target_offset + i);
        if (vm_get_bios(vm)) {
            bios_poke(vm_get_bios(vm), phys_addr, b);
        }
#ifndef BASIC_LITE_BUILD
        else if (vmem) {
            vmem_poke(vmem, (uint16_t)(target_offset + i), b);
        }
#endif
    }

    platform_file_close(fp);
    str_release(vm_get_str(vm), fn_val.as.string);
    return err;
}

// ---- Header Layout Table ----
// 7-Byte Vintage Header (BSaveHeader):
// [00]     Magic Byte (0xFD)
// [01..02] Segment Address (16-bit little-endian)
// [03..04] Offset Address (16-bit little-endian)
// [05..06] Byte Length (16-bit little-endian, up to 65535 bytes)
