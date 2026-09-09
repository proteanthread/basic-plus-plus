// FILENAME: hypot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (hypot.h, math.c, string.c)
// Provides runtime implementation for the HYPOT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/hypot.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_hypot_desc = {
    .name = "HYPOT",
    .category = "Math Functions",
    .syntax = "HYPOT(val1, val2 [, ...]) or val1 HYPOT val2",
    .description = "Returns the Euclidean norm runtime_sqrt(sum of squares) of arguments (supports dual prefix & infix notation).",
    .error_summary = "Error 13: Type Mismatch (HYPOT expects numeric arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_hypot_register(void) {
    lang_desc_register(&g_hypot_desc);
}

BValue func_hypot_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_HYPOT") != 0 && runtime_strcmp(uname, "HYPOT") != 0 && runtime_strcmp(uname, "MATH.HYPOT") != 0) {
        return res;
    }

    if (arg_count < 2) {
        err->code = 13;
        err->message = "HYPOT expects at least two numeric arguments";
        return res;
    }

    double sum_sq = 0.0;
    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            err->code = 13;
            err->message = "HYPOT expects numeric arguments";
            return res;
        }
        sum_sq += args[i].as.number * args[i].as.number;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_sqrt(sum_sq);
    return res;
}
