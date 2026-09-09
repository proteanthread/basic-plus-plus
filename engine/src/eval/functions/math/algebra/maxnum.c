// FILENAME: maxnum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, maxnum.h, string.c)
// Provides runtime implementation for the MAXNUM built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/maxnum.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_maxnum_desc = {
    .name = "MAXNUM",
    .category = "Math Functions",
    .syntax = "MAXNUM",
    .description = "Returns maximum positive representable finite number (ANSI Full BASIC 1987).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_maxnum_register(void) {
    lang_desc_register(&g_maxnum_desc);
}

BValue func_maxnum_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)args;
    (void)err;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "MAXNUM") != 0 && runtime_strcmp(uname, "_MAXNUM") != 0 && runtime_strcmp(uname, "MATH.MAXNUM") != 0) {
        return res;
    }

    if (arg_count != 0) {
        err->code = 5;
        err->message = "MAXNUM expects zero arguments";
        return res;
    }

    res.as.number = 1.7976931348623157e+308;
    res.type = VAL_NUMBER;

    return res;
}
