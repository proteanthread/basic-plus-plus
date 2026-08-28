// FILENAME: void.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, map.h, map.c, vm.h)
// NEEDS: libengine (void.h)
// Provides runtime implementation for the VOID statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/void.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/map.h"
#include "runtime/micro_lib_metadata.h"

void stmt_void_register(void) {
    static const MicroLibMetadata meta = {
        .name = "VOID",
        .category = "Control Flow",
        .syntax = "VOID expression",
        .help_text = "Evaluates an expression or function for side-effects and discards the return value.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_void_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    // Safely discard and release result
    if (val.type == VAL_STRING && val.as.string) {
        str_release(vm_get_str(vm), val.as.string);
    } else if (val.type == VAL_MAP && val.as.map) {
        map_release(vm_get_str(vm), val.as.map);
    }

    return err;
}
