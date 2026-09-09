// FILENAME: avg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (avg.h)
// Provides runtime implementation for the AVG / MEAN built-in function in BASIC++.

#include "eval/functions/math/algebra/avg.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"

static const LangDesc g_avg_desc = {
    .name = "AVG",
    .category = "Math & Statistics Functions",
    .syntax = "AVG(val1, val2 [, ...]) or AVG(arr) or AVG{...} or AVG[arr]",
    .description = "Returns the arithmetic mean of numeric arguments, set elements, or an array (aliases: MEAN).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_mean_desc = {
    .name = "MEAN",
    .category = "Math & Statistics Functions",
    .syntax = "MEAN(val1, val2 [, ...]) or MEAN(arr) or MEAN{...} or MEAN[arr]",
    .description = "Returns the arithmetic mean of numeric arguments, set elements, or an array (alias for AVG).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_avg_register(void) {
    lang_desc_register(&g_avg_desc);
    lang_desc_register(&g_mean_desc);
}

BValue func_avg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_AVG") != 0 && runtime_strcmp(uname, "AVG") != 0 &&
        runtime_strcmp(uname, "MATH.AVG") != 0 && runtime_strcmp(uname, "MEAN") != 0 &&
        runtime_strcmp(uname, "_MEAN") != 0 && runtime_strcmp(uname, "MATH.MEAN") != 0) {
        return res;
    }

    if (arg_count < 1) {
        err->code = 13;
        err->message = "AVG expects at least one numeric argument";
        return res;
    }

    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            err->code = 13;
            err->message = "AVG expects numeric arguments";
            return res;
        }
        sum += args[i].as.number;
        count++;
    }

    res.type = VAL_NUMBER;
    res.as.number = (count > 0) ? (sum / (double)count) : 0.0;
    return res;
}
