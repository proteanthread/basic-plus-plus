// FILENAME: runtime_log.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_log.h, math.c, string.c)
// Provides runtime implementation for the LOG built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/log.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_log_desc = {
    .name = "LOG",
    .category = "Math Functions",
    .syntax = "LOG(x)",
    .description = "Returns the natural logarithm (base e) of a numeric expression x > 0.",
    .error_summary = "Error 5: Illegal Function Call (LOG of zero or negative number), Error 13: Type Mismatch (LOG expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_log_register(void) {
    lang_desc_register(&g_log_desc);
}

BValue func_log_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "LOG") != 0 && runtime_strcmp(uname, "LN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "LOG expects one numeric argument";
        return res;
    }

    if (args[0].as.number <= 0.0) {
        err->code = 5;
        err->message = "LOG of zero or negative number";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_log(args[0].as.number);
    return res;
}
