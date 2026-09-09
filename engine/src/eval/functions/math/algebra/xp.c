// FILENAME: xp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (xp.h)
// Provides runtime implementation for the XP (Exponent Part) function in BASIC++.

#include "eval/functions/math/algebra/xp.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/math/math.h"

static const LangDesc g_xp_desc = {
    .name = "XP",
    .category = "Math Functions",
    .syntax = "XP(num)",
    .description = "Returns the integer base-10 exponent of a number in scientific notation (JOSS).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_xp_register(void) {
    lang_desc_register(&g_xp_desc);
}

BValue func_xp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "XP expects 1 numeric argument";
        return res;
    }

    double v = args[0].as.number;
    if (v == 0.0) {
        res.as.number = 0.0;
        return res;
    }

    double abs_v = runtime_fabs(v);
    res.as.number = runtime_floor(log10(abs_v));
    return res;
}
