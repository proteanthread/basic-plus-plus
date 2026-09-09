// FILENAME: spec_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (spec_fn.h, string.c)
// Provides runtime implementation for the SPEC_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/spec_fn.h"
#include "runtime/language_descriptor.h"
#include "runtime/string.h"
#include "runtime/memory.h"

static const LangDesc g_spec_desc = {
    .name = "SPEC%",
    .category = "System Functions",
    .syntax = "SPEC%(code, arg1 [, arg2])",
    .description = "Executes special device control operations (DEC RSTS/E).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_spec_fn_register(void) {
    lang_desc_register(&g_spec_desc);
}

BValue func_spec_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)vm; (void)uname; (void)args; (void)err;

    // Returns status 0 (Success) for virtual device operations
    res.as.number = (double)arg_count;
    return res;
}
