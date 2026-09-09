// FILENAME: fpt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h, string.h)
// NEEDS: libengine (fpt.h)
// Provides runtime implementation for the FPT (Fractional Part) function in BASIC++.

#include "eval/functions/math/algebra/fpt.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_fpt_desc = {
    .name = "FPT",
    .category = "Math Functions",
    .syntax = "FPT(num) / FP(num)",
    .description = "Returns the fractional part of a number (Business BASIC / BBx).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_fpt_register(void) {
    lang_desc_register(&g_fpt_desc);
}

BValue func_fpt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "FPT") != 0 && runtime_strcmp(uname, "_FPT") != 0 &&
        runtime_strcmp(uname, "MATH.FPT") != 0 && runtime_strcmp(uname, "FP") != 0 &&
        runtime_strcmp(uname, "_FP") != 0 && runtime_strcmp(uname, "MATH.FP") != 0 &&
        runtime_strcmp(uname, "FRAC") != 0 && runtime_strcmp(uname, "_FRAC") != 0 &&
        runtime_strcmp(uname, "MATH.FRAC") != 0) {
        return res;
    }

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "FP/FPT expects 1 numeric argument";
        return res;
    }

    double v = args[0].as.number;
    double intpart;
    res.as.number = runtime_modf(v, &intpart);
    return res;
}
