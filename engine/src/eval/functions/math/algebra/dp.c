// FILENAME: dp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (dp.h)
// Provides runtime implementation for the DP (Digit Part) function in BASIC++.

#include "eval/functions/math/algebra/dp.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/math/math.h"

static const LangDesc g_dp_desc = {
    .name = "DP",
    .category = "Math Functions",
    .syntax = "DP(num)",
    .description = "Returns the normalized digit part (significand in [1, 10)) of a number in scientific notation (JOSS).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_dp_register(void) {
    lang_desc_register(&g_dp_desc);
}

BValue func_dp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "DP expects 1 numeric argument";
        return res;
    }

    double v = args[0].as.number;
    if (v == 0.0) {
        res.as.number = 0.0;
        return res;
    }

    double abs_v = runtime_fabs(v);
    double exp_part = runtime_floor(log10(abs_v));
    res.as.number = v / runtime_pow(10.0, exp_part);
    return res;
}
