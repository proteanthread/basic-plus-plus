// FILENAME: invoke.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, invoke.h, string.c)
// NEEDS: libkernel (security.h, security.c)
// Provides runtime implementation for the INVOKE statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/environment/invoke.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_invoke_desc = {
    .name = "INVOKE",
    .category = "System & Modules",
    .syntax = "INVOKE driver_path$",
    .description = "Apple /// Business BASIC dynamic driver and module loader.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 53: File Not Found",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_invoke_register(void) {
    lang_desc_register(&g_invoke_desc);
}

BppError stmt_invoke_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (security_check(SECOP_MODULE, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: INVOKE blocked by sandbox settings";
        return err;
    }


    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_STRING) {
        err.code = 13;
        err.message = "Type mismatch: INVOKE expects string filename argument";
        return err;
    }

    // Release temporary string after consuming
    str_release(vm_get_str(vm), val.as.string);
    return err;
}
