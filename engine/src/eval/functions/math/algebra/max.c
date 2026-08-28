// FILENAME: max.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, max.h, string.c)
// Provides runtime implementation for the MAX built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/max.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_max_register(void) {
    MicroLibMetadata meta = {
        .name = "MAX",
        .category = "Math Functions",
        .syntax = "MAX(val1, val2 [, ...]) or val1 MAX val2",
        .help_text = "Returns the maximum of two or more numeric values (supports dual prefix & infix notation).",
        .error_codes = "Error 13: Type Mismatch (MAX expects numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_max_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_MAX") != 0 && runtime_strcmp(uname, "MAX") != 0 && runtime_strcmp(uname, "MATH.MAX") != 0) {
        return res;
    }

    if (arg_count < 2) {
        err->code = 13;
        err->message = "MAX expects at least two numeric arguments";
        return res;
    }

    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            err->code = 13;
            err->message = "MAX expects numeric arguments";
            return res;
        }
    }

    double max_val = args[0].as.number;
    for (int i = 1; i < arg_count; i++) {
        if (args[i].as.number > max_val) {
            max_val = args[i].as.number;
        }
    }

    res.type = VAL_NUMBER;
    res.as.number = max_val;
    return res;
}
