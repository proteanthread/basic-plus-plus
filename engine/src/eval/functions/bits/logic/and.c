// FILENAME: and.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (and.h)
// Provides runtime implementation for the AND built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/logic/and.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_and_desc = {
    .name = "AND",
    .category = "Bitwise & Logical Functions",
    .syntax = "AND(val1, val2 [, ...]) or val1 AND val2",
    .description = "Performs bitwise and logical AND conjunction on integers or boolean values (supports dual prefix & infix notation).",
    .error_summary = "Error 13: Type Mismatch (AND expects numeric arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_and_register(void) {
    lang_desc_register(&g_and_desc);
}

BValue func_and_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_AND") != 0 && runtime_strcmp(uname, "AND") != 0 && runtime_strcmp(uname, "BITS.AND") != 0) {
        return res;
    }


    if (arg_count < 2) {
        err->code = 13;
        err->message = "AND expects at least two numeric arguments";
        return res;
    }

    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            err->code = 13;
            err->message = "AND expects numeric arguments";
            return res;
        }
    }

    int64_t accum = (int64_t)args[0].as.number;
    for (int i = 1; i < arg_count; i++) {
        accum &= (int64_t)args[i].as.number;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)accum;
    return res;
}
