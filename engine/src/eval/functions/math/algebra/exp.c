// FILENAME: runtime_exp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_exp.h, math.c, string.c)
// Provides runtime implementation for the EXP built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/exp.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_exp_desc = {
    .name = "EXP",
    .category = "Math Functions",
    .syntax = "EXP(x)",
    .description = "Returns e raised to the power of a numeric expression x.",
    .error_summary = "Error 6: Overflow (EXP exponent too large), Error 13: Type Mismatch (EXP expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_exp_register(void) {
    lang_desc_register(&g_exp_desc);
}

BValue func_exp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "EXP") != 0 && runtime_strcmp(uname, "EXN") != 0 &&
        runtime_strcmp(uname, "_EXP") != 0 && runtime_strcmp(uname, "MATH.EXP") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "EXP expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_exp(args[0].as.number);
    return res;
}
