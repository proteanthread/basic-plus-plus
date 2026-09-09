// FILENAME: int.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (int.h, math.c, string.c)
// Provides runtime implementation for the INT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/int.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_int_desc = {
    .name = "INT",
    .category = "Math Functions",
    .syntax = "INT(x)",
    .description = "Returns the largest integer less than or equal to x (runtime_floor conversion).",
    .error_summary = "Error 13: Type Mismatch (INT expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_int_register(void) {
    lang_desc_register(&g_int_desc);
}

BValue func_int_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "INT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "INT expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_floor(args[0].as.number);
    return res;
}
