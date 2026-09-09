// FILENAME: bitfield.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (bitfield.h)
// Provides runtime implementation for the BITFIELD built-in function in BASIC++.

#include "eval/functions/bits/manipulation/bitfield.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_bitfield_desc = {
    .name = "BITFIELD",
    .category = "Bitwise & Logical Functions",
    .syntax = "BITFIELD(val, start, len) or BITFIELD[val, start, len]",
    .description = "Extracts a bitfield of length len starting at zero-based bit index start.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_bitfield_register(void) {
    lang_desc_register(&g_bitfield_desc);
}

BValue func_bitfield_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_BITFIELD") != 0 && runtime_strcmp(uname, "BITFIELD") != 0 &&
        runtime_strcmp(uname, "BITS.FIELD") != 0) {
        return res;
    }

    if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "BITFIELD expects three numeric arguments";
        return res;
    }

    uint64_t val = (uint64_t)(int64_t)args[0].as.number;
    int64_t start = (int64_t)args[1].as.number;
    int64_t len = (int64_t)args[2].as.number;

    if (start < 0) start = 0;
    if (len <= 0) {
        res.type = VAL_NUMBER;
        res.as.number = 0.0;
        return res;
    }

    uint64_t mask;
    if (len >= 64) {
        mask = ~0ULL;
    } else {
        mask = ((uint64_t)1 << (uint64_t)len) - 1;
    }

    uint64_t extracted = 0;
    if (start < 64) {
        extracted = (val >> (uint64_t)start) & mask;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)extracted;
    return res;
}
