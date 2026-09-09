// FILENAME: not.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (not.h)
// Provides runtime implementation for the NOT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/logic/not.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_not_desc = {
    .name = "NOT",
    .category = "Bitwise & Logical Functions",
    .syntax = "NOT(val) or NOT val",
    .description = "Performs bitwise and logical negation on an integer or boolean value.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_not_register(void) {
    lang_desc_register(&g_not_desc);
}

BValue func_not_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_NOT") != 0 && runtime_strcmp(uname, "NOT") != 0 && runtime_strcmp(uname, "BITS.NOT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "NOT expects one numeric argument";
        return res;
    }

    int64_t a = (int64_t)args[0].as.number;

    res.type = VAL_NUMBER;
    res.as.number = (double)(~a);
    return res;
}
