// FILENAME: log10.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (log10.h, math.c, string.c)
// Provides runtime implementation for the LOG10 built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/log10.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_log10_register(void) {
    static const MicroLibMetadata meta = {
        .name = "LOG10",
        .category = "Math & Trigonometry",
        .syntax = "LOG10(x) | LGT(x)",
        .help_text = "Returns the base-10 logarithm of x (x > 0).",
        .error_codes = "Error 5: Illegal Function Call (x <= 0), Error 13: Type Mismatch (LOG10 expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_log10_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "LOG10") != 0 && runtime_strcmp(uname, "LGT") != 0 &&
        runtime_strcmp(uname, "_LOG10") != 0 && runtime_strcmp(uname, "MATH.LOG10") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "LOG10 expects one numeric argument";
        return res;
    }

    if (args[0].as.number <= 0.0) {
        err->code = 5;
        err->message = "Illegal function call: LOG10 argument must be positive";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_log10(args[0].as.number);
    return res;
}
