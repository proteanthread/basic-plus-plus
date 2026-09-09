// FILENAME: csc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (csc.h, math.c, string.c)
// Provides runtime implementation for the CSC built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/csc.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_csc_desc = {
    .name = "CSC",
    .category = "Math & Trigonometry",
    .syntax = "CSC(x)",
    .description = "Returns the cosecant of angle x in radians (1 / runtime_sin(x)).",
    .error_summary = "Error 11: Division by Zero (runtime_sin(x) == 0), Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_csc_register(void) {
    lang_desc_register(&g_csc_desc);
}

BValue func_csc_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "CSC") != 0 && runtime_strcmp(uname, "_CSC") != 0 && runtime_strcmp(uname, "MATH.CSC") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "CSC expects one numeric argument";
        return res;
    }

    double s = runtime_sin(args[0].as.number);
    if (s == 0.0) {
        err->code = 11;
        err->message = "Division by zero in CSC";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = 1.0 / s;
    return res;
}
