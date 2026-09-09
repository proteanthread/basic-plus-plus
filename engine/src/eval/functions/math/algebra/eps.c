// FILENAME: eps.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (eps.h, math.c, string.c)
// Provides runtime implementation for the EPS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/eps.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_eps_desc = {
    .name = "EPS",
    .category = "Math Functions",
    .syntax = "EPS(x)",
    .description = "Returns the machine epsilon relative to x (ANSI Full BASIC 1987).",
    .error_summary = "Error 13: Type Mismatch (EPS expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_eps_register(void) {
    lang_desc_register(&g_eps_desc);
}

BValue func_eps_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "EPS") != 0 && runtime_strcmp(uname, "_EPS") != 0 && runtime_strcmp(uname, "MATH.EPS") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "EPS expects one numeric argument";
        return res;
    }

    double x = runtime_fabs(args[0].as.number);
    if (x == 0.0) {
        res.as.number = RUNTIME_DBL_EPSILON;
    } else {
        res.as.number = x * RUNTIME_DBL_EPSILON;
    }
    res.type = VAL_NUMBER;


    return res;
}
