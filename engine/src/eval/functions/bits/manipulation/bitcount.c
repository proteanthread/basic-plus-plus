// FILENAME: bitcount.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (bitcount.h)
// Provides runtime implementation for the BITCOUNT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/manipulation/bitcount.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_bitcount_desc = {
    .name = "BITCOUNT",
    .category = "Bitwise & Logical Functions",
    .syntax = "BITCOUNT(val)",
    .description = "Returns the population count (number of set bits) of an integer value.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_bitcount_register(void) {
    lang_desc_register(&g_bitcount_desc);
}

BValue func_bitcount_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_BITCOUNT") != 0 && runtime_strcmp(uname, "BITCOUNT") != 0 && runtime_strcmp(uname, "BITS.COUNT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "BITCOUNT expects one numeric argument";
        return res;
    }

    uint64_t temp = (uint64_t)(int64_t)args[0].as.number;
    int count = 0;
    while (temp) {
        if (temp & 1) count++;
        temp >>= 1;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)count;
    return res;
}
