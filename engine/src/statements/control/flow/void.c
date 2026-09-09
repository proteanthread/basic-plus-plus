// FILENAME: void.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
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
#include "runtime/language_descriptor.h"

static const LangDesc g_void_desc = {
    .name = "VOID",
    .category = "Control Flow",
    .syntax = "VOID expression",
    .description = "Evaluates an expression or function for side-effects and discards the return value.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_void_register(void) {
    lang_desc_register(&g_void_desc);
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
