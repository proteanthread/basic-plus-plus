// FILENAME: eqv.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (eqv.h)
// Provides runtime implementation for the EQV built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/logic/eqv.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_eqv_desc = {
    .name = "EQV",
    .category = "Bitwise & Logical Functions",
    .syntax = "EQV(val1, val2) or val1 EQV val2",
    .description = "Performs bitwise and logical equivalence (NOT XOR) on integers or boolean values.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_eqv_register(void) {
    lang_desc_register(&g_eqv_desc);
}

BValue func_eqv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_EQV") != 0 && runtime_strcmp(uname, "EQV") != 0 && runtime_strcmp(uname, "BITS.EQV") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "EQV expects two numeric arguments";
        return res;
    }

    int64_t a = (int64_t)args[0].as.number;
    int64_t b = (int64_t)args[1].as.number;

    res.type = VAL_NUMBER;
    res.as.number = (double)(~(a ^ b));
    return res;
}
