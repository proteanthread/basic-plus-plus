// FILENAME: ror.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (ror.h)
// Provides runtime implementation for the ROR built-in function in BASIC++.

#include "eval/functions/bits/shift/ror.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_ror_desc = {
    .name = "ROR",
    .category = "Bitwise & Logical Functions",
    .syntax = "ROR(val, count [, width])",
    .description = "Rotates bits of an integer right by the specified count within the given bit width (default: 64).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_ror_register(void) {
    lang_desc_register(&g_ror_desc);
}

BValue func_ror_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_ROR") != 0 && runtime_strcmp(uname, "ROR") != 0 &&
        runtime_strcmp(uname, "BITS.ROR") != 0 && runtime_strcmp(uname, "ROTR") != 0) {
        return res;
    }

    if (arg_count < 2 || arg_count > 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "ROR expects two or three numeric arguments";
        return res;
    }

    uint64_t val = (uint64_t)(int64_t)args[0].as.number;
    uint64_t count = (uint64_t)(int64_t)args[1].as.number;
    int width = 64;
    if (arg_count == 3 && args[2].type != VAL_STRING) {
        width = (int)args[2].as.number;
        if (width != 8 && width != 16 && width != 32 && width != 64) {
            width = 64;
        }
    }

    uint64_t out_val = 0;
    if (width == 8) {
        count %= 8;
        uint8_t v = (uint8_t)val;
        out_val = (count == 0) ? v : (uint8_t)((v >> count) | (v << (8 - count)));
    } else if (width == 16) {
        count %= 16;
        uint16_t v = (uint16_t)val;
        out_val = (count == 0) ? v : (uint16_t)((v >> count) | (v << (16 - count)));
    } else if (width == 32) {
        count %= 32;
        uint32_t v = (uint32_t)val;
        out_val = (count == 0) ? v : (uint32_t)((v >> count) | (v << (32 - count)));
    } else {
        count %= 64;
        out_val = (count == 0) ? val : ((val >> count) | (val << (64 - count)));
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)out_val;
    return res;
}
