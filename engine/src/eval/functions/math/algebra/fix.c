// FILENAME: fix.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (fix.h, math.c, string.c)
// Provides runtime implementation for the FIX built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/fix.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_fix_desc = {
    .name = "FIX",
    .category = "Math Functions",
    .syntax = "FIX(x)",
    .description = "Returns the truncated integer part of x (truncates towards zero).",
    .error_summary = "Error 13: Type Mismatch (FIX expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_fix_register(void) {
    lang_desc_register(&g_fix_desc);
}

BValue func_fix_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "FIX") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "FIX expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (args[0].as.number >= 0.0) ? runtime_floor(args[0].as.number) : runtime_ceil(args[0].as.number);
    return res;
}
