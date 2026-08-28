// FILENAME: out.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (bios.h, bios.c, eval.h, eval.c, out.h, string.c)
// NEEDS: libkernel (bus.h, bus.c, security.h, security.c)
// Provides runtime implementation for the OUT statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/out.h"
#include "eval/eval.h"
#include "device/bus.h"
#include "bios/bios.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_out_register(void) {
    static const MicroLibMetadata meta = {
        .name = "OUT",
        .category = "System & Hardware I/O",
        .syntax = "OUT port, data",
        .help_text = "Sends a byte (0-255) to a hardware or virtual I/O port address.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_out_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue pval = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (pval.type != VAL_NUMBER && pval.type != VAL_INTEGER) {
        if (pval.type == VAL_STRING && pval.as.string) {
            str_release(vm_get_str(vm), pval.as.string);
        }
        err.code = 13; err.message = "Type mismatch (expected numeric port for OUT)";
        return err;
    }

    int port = (int)pval.as.number;
    if (port < 0 || port > 65535) {
        err.code = 5; err.message = "Illegal function call (port out of bounds 0-65535)";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' between port and data in OUT";
        return err;
    }

    BValue dval = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (dval.type != VAL_NUMBER && dval.type != VAL_INTEGER) {
        if (dval.type == VAL_STRING && dval.as.string) {
            str_release(vm_get_str(vm), dval.as.string);
        }
        err.code = 13; err.message = "Type mismatch (expected numeric data for OUT)";
        return err;
    }

    int val = (int)dval.as.number;
    if (val < -128 || val > 255) {
        err.code = 5; err.message = "Illegal function call (byte value out of range 0-255)";
        return err;
    }
    uint8_t byte_val = (uint8_t)(val & 0xFF);

    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70; err.message = "Permission denied: Port I/O is restricted";
        return err;
    }

    vdev_bus_out(port, byte_val);
    BiosContext *bios = vm_get_bios(vm);
    if (bios) {
        bios_out(bios, (uint16_t)port, byte_val);
    }

    return err;
}
