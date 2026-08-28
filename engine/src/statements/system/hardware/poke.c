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
#include "runtime/micro_lib_metadata.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include <string.h>
#include <stdint.h>

BppError stmt_poke_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

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

    // Evaluate second argument: byte value
    BValue byte_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (byte_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), byte_val.as.string);
        err.code = 13;
        err.message = "Type mismatch: POKE expects numeric value";
        return err;
    }

    // Validate byte range: 0..255
    int val = (int)byte_val.as.number;
    if (val < 0 || val > 255) {
        err.code = 5;
        err.message = "Illegal function call: POKE value must be 0-255";
        return err;
    }

    // Compute physical address using DEF SEG if available
    uint32_t addr_raw = (uint32_t)addr_val.as.number;
    uint32_t phys_addr = addr_raw;
#ifndef BASIC_LITE_BUILD
    uint16_t def_seg = vmem_get_def_seg(vm_get_vmem(vm));
    if (def_seg != 0 && addr_raw < 0x10000U) {
        phys_addr = ((uint32_t)def_seg << 4) + addr_raw;
    }
#endif

    // Intercept ESP32 hardware register address space
    if (esp32_is_hardware_addr(phys_addr)) {
        esp32_reg_write8(phys_addr, (uint8_t)val);
        return err;
    }

    // Write byte through BIOS (triggers VRAM observer) or device bus fallback
    if (vm_get_bios(vm)) {
        bios_poke(vm_get_bios(vm), phys_addr, (uint8_t)val);
    } else {
        bool intercepted = false;
        vdev_bus_poke(phys_addr, (uint8_t)val, &intercepted);
    }

    return err;
}


void stmt_poke_register(void) {
    static const MicroLibMetadata meta = {
        .name = "POKE",
        .category = "Variables & Memory",
        .syntax = "POKE offset, byte_val | OUT port, val",
        .help_text = "Writes a byte directly to virtual memory at segment:offset or to an I/O port.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

