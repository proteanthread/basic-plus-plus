// FILENAME: remainder.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, remainder.h, string.c)
// Provides runtime implementation for the REMAINDER built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/remainder.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_remainder_desc = {
    .name = "REMAINDER",
    .category = "Math Functions",
    .syntax = "REMAINDER(x, y)",
    .description = "Returns the remainder of x divided by y (ANSI Full BASIC 1987).",
    .error_summary = "Error 11: Division by zero, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_remainder_register(void) {
    lang_desc_register(&g_remainder_desc);
}

BValue func_remainder_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "REMAINDER") != 0 && runtime_strcmp(uname, "_REMAINDER") != 0 && runtime_strcmp(uname, "MATH.REMAINDER") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "REMAINDER expects two numeric arguments";
        return res;
    }

    double x = args[0].as.number;
    double y = args[1].as.number;

    if (y == 0.0) {
        err->code = 11;
        err->message = "Division by zero in REMAINDER";
        return res;
    }

    res.as.number = x - y * runtime_floor(x / y);
    res.type = VAL_NUMBER;

    return res;
}
