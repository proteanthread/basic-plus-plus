// FILENAME: scale.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, scale.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the SCALE statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/external/scale.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>

void stmt_scale_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SCALE",
        .category = "Numeric & Math",
        .syntax = "SCALE factor",
        .help_text = "Sets fixed-point decimal scale factor (0 to 6) for arithmetic precision and rounding.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_scale_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_NUMBER && val.type != VAL_INTEGER) {
        if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    int factor = (int)val.as.number;
    if (factor < 0 || factor > 6) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        err.message = "SCALE factor must be between 0 and 6";
        return err;
    }

    return err;
}
