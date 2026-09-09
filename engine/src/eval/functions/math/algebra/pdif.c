// FILENAME: pdif.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, pdif.h, string.c)
// Provides runtime implementation for the PDIF built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/pdif.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_pdif_desc = {
    .name = "PDIF",
    .category = "Math Functions",
    .syntax = "PDIF(a, b)",
    .description = "Returns the positive difference of a and b (a - b if a > b, else 0).",
    .error_summary = "Error 13: Type Mismatch (PDIF expects two numeric arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_pdif_register(void) {
    lang_desc_register(&g_pdif_desc);
}

BValue func_pdif_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "PDIF") != 0 && runtime_strcmp(uname, "_PDIF") != 0 && runtime_strcmp(uname, "MATH.PDIF") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "PDIF expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    double a = args[0].as.number;
    double b = args[1].as.number;
    res.as.number = (a > b) ? (a - b) : 0.0;
    return res;
}
