// FILENAME: sgn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, sgn.h, string.c)
// Provides runtime implementation for the SGN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/sgn.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_sgn_desc = {
    .name = "SGN",
    .category = "Math Functions",
    .syntax = "SGN(x)",
    .description = "Returns the sign of x: 1 if x > 0, 0 if x = 0, -1 if x < 0.",
    .error_summary = "Error 13: Type Mismatch (SGN expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_sgn_register(void) {
    lang_desc_register(&g_sgn_desc);
}

BValue func_sgn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "SGN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SGN expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    double val = args[0].as.number;
    if (val > 0.0) res.as.number = 1.0;
    else if (val < 0.0) res.as.number = -1.0;
    else res.as.number = 0.0;
    return res;
}
