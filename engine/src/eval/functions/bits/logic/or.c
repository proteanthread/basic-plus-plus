// FILENAME: or.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (or.h)
// Provides runtime implementation for the OR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/logic/or.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_or_desc = {
    .name = "OR",
    .category = "Bitwise & Logical Functions",
    .syntax = "OR(val1, val2 [, ...]) or val1 OR val2",
    .description = "Performs bitwise and logical inclusive OR disjunction on integers or boolean values.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_or_register(void) {
    lang_desc_register(&g_or_desc);
}

BValue func_or_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_OR") != 0 && runtime_strcmp(uname, "OR") != 0 && runtime_strcmp(uname, "BITS.OR") != 0) {
        return res;
    }

    if (arg_count < 2) {
        err->code = 13;
        err->message = "OR expects at least two numeric arguments";
        return res;
    }

    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            err->code = 13;
            err->message = "OR expects numeric arguments";
            return res;
        }
    }

    int64_t accum = (int64_t)args[0].as.number;
    for (int i = 1; i < arg_count; i++) {
        accum |= (int64_t)args[i].as.number;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)accum;
    return res;
}
