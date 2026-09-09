// FILENAME: poke.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the POKE statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/poke.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "device/bus.h"
#include "bios/bios.h"
#include "security/security.h"
#include "esp32_regs.h"
#include "runtime/language_descriptor.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/strings.h"
#include "statements/system/hardware/def_seg.h"
#include <stdint.h>

static const LangDesc g_poke_desc = {
    .name = "POKE",
    .category = "Variables & Memory",
    .syntax = "POKE offset, byte_val | OUT port, val",
    .description = "Writes a byte directly to virtual memory at segment:offset or to an I/O port.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

static void poke_byte_internal(VMContext *vm, uint32_t phys_addr, uint8_t byte_val) {
    if (esp32_is_hardware_addr(phys_addr)) {
        esp32_reg_write8(phys_addr, byte_val);
    } else if (vm_get_bios(vm)) {
        bios_poke(vm_get_bios(vm), phys_addr, byte_val);
    } else {
        bool intercepted = false;
        vdev_bus_poke(phys_addr, byte_val, &intercepted);
    }
}

BppError stmt_poke_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    // Security sandbox: require memory write permission
    if (security_check(SECOP_MEM_WRITE, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: POKE is restricted";
        return err;
    }

    // Evaluate first argument: address/offset
    BValue addr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (addr_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), addr_val.as.string);
        err.code = 13;
        err.message = "Type mismatch: POKE expects numeric address";
        return err;
    }

    // Expect comma separator
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2;
        err.message = "Syntax error: Expected comma after POKE address";
        return err;
    }
    lex_next(lex); // consume comma

    // Evaluate second argument: value or string
    BValue byte_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    uint32_t addr_raw = (uint32_t)addr_val.as.number;
    uint32_t phys_addr = addr_raw;
    uint16_t def_seg = 0;
#ifndef BASIC_LITE_BUILD
    if (vm_get_vmem(vm)) def_seg = vmem_get_def_seg(vm_get_vmem(vm));
#endif
    if (def_seg == 0) def_seg = runtime_get_def_seg();
    if (def_seg != 0 && addr_raw < 0x10000U) {
        phys_addr = ((uint32_t)def_seg << 4) + addr_raw;
    }

    // String block write
    if (byte_val.type == VAL_STRING) {
        if (byte_val.as.string) {
            const char *sdata = str_data(byte_val.as.string);
            size_t slen = str_len(byte_val.as.string);
            for (size_t k = 0; k < slen; k++) {
                poke_byte_internal(vm, phys_addr + (uint32_t)k, (uint8_t)sdata[k]);
            }
            str_release(vm_get_str(vm), byte_val.as.string);
        }
        return err;
    }

    int width = 1;
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        BValue w_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (w_val.type == VAL_STRING) {
            str_release(vm_get_str(vm), w_val.as.string);
        } else {
            width = (int)w_val.as.number;
            if (width != 1 && width != 2 && width != 4 && width != 8) width = 1;
        }
    }

    uint64_t val = (uint64_t)byte_val.as.number;
    if (width == 1) {
        if ((int64_t)byte_val.as.number < 0 || (int64_t)byte_val.as.number > 255) {
            err.code = 5;
            err.message = "Illegal function call: POKE value must be 0-255";
            return err;
        }
        poke_byte_internal(vm, phys_addr, (uint8_t)(val & 0xFF));
    } else {
        for (int k = 0; k < width; k++) {
            uint8_t b = (uint8_t)((val >> (k * 8)) & 0xFF);
            poke_byte_internal(vm, phys_addr + (uint32_t)k, b);
        }
    }

    return err;
}


void stmt_poke_register(void) {
    lang_desc_register(&g_poke_desc);
}

