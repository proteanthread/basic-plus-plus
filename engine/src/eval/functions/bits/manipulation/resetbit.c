// FILENAME: resetbit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (resetbit.h)
// Provides runtime implementation for the RESETBIT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/manipulation/resetbit.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_resetbit_desc = {
    .name = "RESETBIT",
    .category = "Bitwise & Logical Functions",
    .syntax = "RESETBIT(val, bit)",
    .description = "Clears (sets to 0) the bit at the specified zero-based bit index in an integer value.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_clrbit_desc = {
    .name = "CLRBIT",
    .category = "Bitwise & Logical Functions",
    .syntax = "CLRBIT(val, bit)",
    .description = "Clears (sets to 0) the bit at the specified zero-based bit index (alias for RESETBIT).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_resetbit_register(void) {
    lang_desc_register(&g_resetbit_desc);
    lang_desc_register(&g_clrbit_desc);
}

BValue func_resetbit_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_RESETBIT") != 0 && runtime_strcmp(uname, "RESETBIT") != 0 &&
        runtime_strcmp(uname, "BITS.RESET") != 0 && runtime_strcmp(uname, "CLRBIT") != 0 &&
        runtime_strcmp(uname, "_CLRBIT") != 0 && runtime_strcmp(uname, "BITS.CLR") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "CLRBIT expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)((uint64_t)(int64_t)args[0].as.number & ~((uint64_t)1 << (uint64_t)(int64_t)args[1].as.number));
    return res;
}
