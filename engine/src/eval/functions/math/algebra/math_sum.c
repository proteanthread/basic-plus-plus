// FILENAME: math_sum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (math_sum.h)
// Provides runtime implementation for the mathematical SUM built-in function in BASIC++.

#include "eval/functions/math/algebra/math_sum.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_math_sum_desc = {
    .name = "SUM",
    .category = "Math & Statistics Functions",
    .syntax = "SUM(val1, val2 [, ...]) or SUM(arr) or SUM{...} or SUM[arr]",
    .description = "Returns the sum of numeric arguments, set elements, or array elements (ANSI Full BASIC / Dartmouth).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_math_sum_register(void) {
    lang_desc_register(&g_math_sum_desc);
}

BValue func_math_sum_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_SUM") != 0 && runtime_strcmp(uname, "SUM") != 0 &&
        runtime_strcmp(uname, "MATH.SUM") != 0) {
        return res;
    }

    if (arg_count < 1) {
        err->code = 13;
        err->message = "SUM expects at least one numeric argument";
        return res;
    }

    double total = 0.0;
    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            err->code = 13;
            err->message = "SUM expects numeric arguments";
            return res;
        }
        total += args[i].as.number;
    }

    res.type = VAL_NUMBER;
    res.as.number = total;
    return res;
}
