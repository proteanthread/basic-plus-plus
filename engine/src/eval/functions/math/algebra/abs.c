// FILENAME: abs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (abs.h, math.c, string.c)
// Provides runtime implementation for the ABS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/abs.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_abs_desc = {
    .name = "ABS",
    .category = "Math Functions",
    .syntax = "ABS(x)",
    .description = "Returns the absolute value of a numeric expression.",
    .error_summary = "Error 13: Type Mismatch (ABS expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_abs_register(void) {
    lang_desc_register(&g_abs_desc);
}

BValue func_abs_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "ABS") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ABS expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    if (args[0].type == VAL_COMPLEX) {
        res.as.number = runtime_hypot(args[0].as.complex_val.real, args[0].as.complex_val.imag);
    } else {
        res.as.number = runtime_fabs(args[0].as.number);
    }
    return res;
}
