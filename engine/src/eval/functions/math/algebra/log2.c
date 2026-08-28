// FILENAME: log2.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (log2.h, math.c, string.c)
// Provides runtime implementation for the LOG2 built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/log2.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_log2_register(void) {
    static const MicroLibMetadata meta = {
        .name = "LOG2",
        .category = "Math & Trigonometry",
        .syntax = "LOG2(x)",
        .help_text = "Returns the base-2 logarithm of x (x > 0).",
        .error_codes = "Error 5: Illegal Function Call (x <= 0), Error 13: Type Mismatch (LOG2 expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_log2_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "LOG2") != 0 && runtime_strcmp(uname, "_LOG2") != 0 && runtime_strcmp(uname, "MATH.LOG2") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "LOG2 expects one numeric argument";
        return res;
    }

    if (args[0].as.number <= 0.0) {
        err->code = 5;
        err->message = "Illegal function call: LOG2 argument must be positive";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_log2(args[0].as.number);
    return res;
}
